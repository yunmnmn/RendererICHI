#include <algorithm>
#include <mutex>
#include <queue>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <TaskScheduler.h>

#include <Util/Util.h>
#include <Util/Logger.h>
#include <IO/FileIO.h>

// GHI abstract interface
#include <GHI/Renderer.h>
#include <GHI/RendererTypes.h>
#include <GHI/RenderResource.h>
#include <GHI/ResourceFactory.h>
#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/CommandBuffer.h>
#include <GHI/CommandRecorder.h>
#include <GHI/SubCommandRecorder.h>
#include <GHI/Fence.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/ShaderModule.h>
#include <GHI/RenderWindow.h>
#include <GHI/Swapchain.h>
#include <GHI/DescriptorPool.h>
#include <GHI/VertexInputState.h>
#include <GHI/RenderCommands.h>
#include <GHI/Device.h>
#include <GHI/PhysicalDevice.h>

// Vulkan-specific (required while GHI abstractions are incomplete in the rework)
#include <GHI/Vulkan/ResourceFactory.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/Swapchain.h>
#include <GHI/Vulkan/VertexInputState.h>

// Renderer state
#include <RendererState.h>
#include <RendererStateInterface.h>

using namespace Foundation;
using namespace Render;
using namespace Render::GHI;

struct Vertex
{
   float position[3] = {};
   float color[3] = {};
};

struct Mvp
{
   glm::mat4 projectionMatrix;
   glm::mat4 modelMatrix;
   glm::mat4 viewMatrix;
};

std::array<Ptr<Buffer>, 2u> CreateVertexAndIndexBuffer(GHI::ResourceFactory& p_factory, Ptr<GHI::Device> p_device)
{
   const std::vector<Vertex> vertices = {{.position = {1.0f, 1.0f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
                                         {.position = {-1.0f, 1.0f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
                                         {.position = {0.0f, -1.0f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}}};
   const uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);

   const std::vector<uint32_t> indices = {0u, 1u, 2u};
   const uint32_t indicesSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);

   Ptr<Buffer> vertexBuffer;
   {
      BufferDescriptor desc;
      desc.m_requestBufferSize = vertexBufferSize;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_bufferUsageFlags = BufferUsageFlags::TransferDestination | BufferUsageFlags::VertexBuffer;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      desc.m_initialData = vertices.data();
      desc.m_initialDataSize = vertexBufferSize;
      vertexBuffer = p_factory.CreateBuffer(p_device, std::move(desc));
   }

   Ptr<Buffer> indexBuffer;
   {
      BufferDescriptor desc;
      desc.m_requestBufferSize = indicesSize;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_bufferUsageFlags = BufferUsageFlags::TransferDestination | BufferUsageFlags::IndexBuffer;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      desc.m_initialData = indices.data();
      desc.m_initialDataSize = indicesSize;
      indexBuffer = p_factory.CreateBuffer(p_device, std::move(desc));
   }

   return {vertexBuffer, indexBuffer};
}

Ptr<GHI::PhysicalDevice> SelectPhysicalDevice(const std::vector<Ptr<GHI::PhysicalDevice>>& p_physicalDevices)
{
   for (const Ptr<GHI::PhysicalDevice>& physicalDevice : p_physicalDevices)
   {
      if (physicalDevice->IsViable() && physicalDevice->GetGPUTypes() == GPUType::Discrete)
      {
         return physicalDevice;
      }
   }
   ASSERT(false, "No viable discrete GPU found");
   return nullptr;
}

void RenderFunction(GHI::ResourceFactory& p_factory)
{
   // Enumerate physical devices (triggers VulkanInstance creation on first call via Get())
   std::vector<Ptr<GHI::PhysicalDevice>> physicalDevices = p_factory.GetPhysicalDevices();
   Ptr<GHI::PhysicalDevice> physicalDevice = SelectPhysicalDevice(physicalDevices);

   // Create the logical device (also internally creates CommandPoolManager and AsyncUploadQueue)
   Ptr<GHI::Device> device = p_factory.CreateDevice(DeviceDescriptor{.m_physicalDevice = physicalDevice});

   // Create the render window
   Ptr<GHI::RenderWindow> renderWindow = p_factory.CreateRenderWindow(
       device, RenderWindowDescriptor{.m_windowResolution = glm::uvec2(1920u, 1080u), .m_windowTitle = "Triangle"});

   // Create the swapchain (surface is created internally from the window native handle)
   Ptr<GHI::Swapchain> swapchain = p_factory.CreateSwapchain(device, SwapchainDescriptor{.m_renderWindow = renderWindow});
   swapchain->Init();

   // Load shader binaries and create ShaderModules
   Ptr<GHI::ShaderModule> vertexShaderModule;
   Ptr<GHI::ShaderModule> fragmentShaderModule;
   {
      using namespace Foundation::IO;

      std::vector<uint8_t> vertexShaderBin;
      std::vector<uint8_t> fragmentShaderBin;

      {
         auto io = FileIO::CreateFileIO(FileIODescriptor{
             .m_path = "Data/Shaders/triangle.vert.spv",
             .m_fileIOFlags = Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});
         io->Open();
         const uint64_t size = io->GetFileSize();
         vertexShaderBin.resize(size);
         io->Read(vertexShaderBin.data(), size);
      }
      {
         auto io = FileIO::CreateFileIO(FileIODescriptor{
             .m_path = "Data/Shaders/triangle.frag.spv",
             .m_fileIOFlags = Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});
         io->Open();
         const uint64_t size = io->GetFileSize();
         fragmentShaderBin.resize(size);
         io->Read(fragmentShaderBin.data(), size);
      }

      vertexShaderModule = p_factory.CreateShaderModule(
          device, ShaderModuleDescriptor{.m_spirvBinary = vertexShaderBin.data(),
                                         .m_binarySizeInBytes = static_cast<uint32_t>(vertexShaderBin.size())});
      fragmentShaderModule = p_factory.CreateShaderModule(
          device, ShaderModuleDescriptor{.m_spirvBinary = fragmentShaderBin.data(),
                                         .m_binarySizeInBytes = static_cast<uint32_t>(fragmentShaderBin.size())});
   }

   // Create vertex and index buffers
   auto buffers = CreateVertexAndIndexBuffer(p_factory, device);
   Ptr<Buffer> vertexBuffer = buffers[0];
   Ptr<Buffer> indexBuffer = buffers[1];

   // Create uniform buffer
   Mvp mvp{.projectionMatrix = glm::mat4(1.0f), .modelMatrix = glm::mat4(1.0f), .viewMatrix = glm::mat4(1.0f)};
   Ptr<Buffer> uniformBuffer;
   {
      BufferDescriptor desc;
      desc.m_requestBufferSize = sizeof(Mvp);
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_bufferUsageFlags = BufferUsageFlags::TransferDestination | BufferUsageFlags::Uniform;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      desc.m_initialData = &mvp;
      desc.m_initialDataSize = sizeof(Mvp);
      uniformBuffer = p_factory.CreateBuffer(device, std::move(desc));
   }

   // Create descriptor pool (replaces the old DescriptorSetLayout + DescriptorSet pair)
   // TODO: The new DescriptorPool API does not yet expose a mechanism to bind specific buffer/image
   //       resources to shader binding slots. That binding path is incomplete in the rework.
   Ptr<DescriptorPool> descriptorPool = p_factory.CreateDescriptorPool(
       device, DescriptorPoolDescriptor{.m_poolType = DescriptorPoolType::Resource, .m_poolSize = 1u});

   // Create the depth/stencil image
   // TODO: ResourceFormat is missing depth/stencil formats (e.g. D32SFloat). Using Invalid as a
   //       placeholder. Add the required formats to the ResourceFormat enum to fix this.
   const glm::uvec2 swapchainExtend = swapchain->GetExtend();
   Ptr<Image> depthStencilImage;
   {
      ImageDescriptor desc;
      desc.m_imageUsageFlags = ImageUsageFlags::DepthStencilAttachment;
      desc.m_imageType = ImageType::Image2D;
      desc.m_extend = glm::uvec3(swapchainExtend.x, swapchainExtend.y, 1u);
      desc.m_format = ResourceFormat::Invalid; // TODO: needs a depth format (e.g. D32SFloat)
      desc.m_mipLevels = 1u;
      desc.m_arrayLayers = 1u;
      desc.m_imageTiling = ImageTiling::TilingOptimal;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_initialLayout = ImageLayout::Undefined;
      depthStencilImage = p_factory.CreateImage(device, std::move(desc));
   }

   Ptr<ImageView> depthStencilImageView;
   {
      ImageViewDescriptor desc;
      desc.m_image = depthStencilImage;
      desc.m_extend = depthStencilImage->GetImageExtend();
      desc.m_viewType = ImageViewType::View2D;
      desc.m_format = ResourceFormat::Invalid; // TODO: needs a depth format
      desc.m_baseMipLevel = 0u;
      desc.m_mipLevelCount = 1u;
      desc.m_baseArrayLayer = 0u;
      desc.m_arrayLayerCount = 1u;
      desc.m_aspectMask = ImageAspectFlags::Depth | ImageAspectFlags::Stencil;
      depthStencilImageView = p_factory.CreateImageView(device, std::move(desc));
   }

   // Configure vertex input state using the Vulkan-specific type directly.
   // TODO: GHI::VertexInputState has a private constructor and no factory method, so it cannot be
   //       instantiated from user code. Vulkan::VertexInputState does not inherit from it, so the
   //       Cast in GraphicsPipeline::GraphicsPipeline() will assert. Fix: have Vulkan::VertexInputState
   //       inherit from GHI::VertexInputState and expose creation through ResourceFactory.
   GHI::Vulkan::VertexInputState vertexInputState;
   {
      GHI::Vulkan::VertexInputBinding& binding = vertexInputState.AddVertexInputBinding(VertexInputRate::VertexInputRateVertex);
      binding.AddVertexInputAttribute(0u, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
      binding.AddVertexInputAttribute(1u, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
   }

   // Create the graphics pipeline
   Ptr<GraphicsPipeline> graphicsPipeline;
   {
      ColorBlendAttachmentState colorBlend = {};
      colorBlend.blendEnable = false;
      colorBlend.srcColorBlendFactor = BlendFactor::FactorZero;
      colorBlend.dstColorBlendFactor = BlendFactor::FactorZero;
      colorBlend.colorBlendOp = BlendOp::Add;
      colorBlend.srcAlphaBlendFactor = BlendFactor::FactorZero;
      colorBlend.dstAlphaBlendFactor = BlendFactor::FactorZero;
      colorBlend.alphaBlendOp = BlendOp::Add;
      colorBlend.colorWriteFlags = ColorComponentFlags::RGBA;

      GraphicsPipelineDescriptor desc;
      desc.m_shaderStages = {
          PipelineShaderStage{.m_shaderModule = vertexShaderModule, .m_shaderStageFlag = ShaderStageFlag::Vertex},
          PipelineShaderStage{.m_shaderModule = fragmentShaderModule, .m_shaderStageFlag = ShaderStageFlag::Fragment}};
      desc.m_layoutBindings = {
          PipelineLayout{.m_binding = 0u,
                         .m_descriptorType = DescriptorType::UniformBuffer,
                         .m_descriptorCount = 1u,
                         .m_stages = PipelineStageFlags::VertexShader}};
      desc.m_vertexInputState = nullptr; // TODO: see VertexInputState note above
      desc.m_polygonMode = PolygonMode::PolygonModeFill;
      desc.m_primitiveTopologyClass = PrimitiveTopologyClass::Triangle;
      desc.m_colorBlendAttachmentStates = {colorBlend};
      desc.m_colorAttachmentFormats = {swapchain->GetFormat()};
      desc.m_depthFormat = ResourceFormat::Invalid;   // TODO: needs depth format
      desc.m_stencilFormat = ResourceFormat::Invalid; // TODO: needs depth format

      graphicsPipeline = p_factory.CreateGraphicsPipeline(device, std::move(desc));
   }

   // Create fences (replaces the old TimelineSemaphore + binary Semaphore pair)
   // submitFence: timeline fence for CPU-GPU frame pacing
   // renderFence: signals when rendering is complete; waited on by presentation
   // acquireFence: signals when the swapchain image is ready to render into
   // NOTE: vkAcquireNextImageKHR expects a binary semaphore, but GHI::Fence wraps a timeline
   //       semaphore. This is a known mismatch in the current rework state.
   Ptr<Fence> submitFence = p_factory.CreateFence(device, FenceDescriptor{.m_initialValue = 0u});
   Ptr<Fence> renderFence = p_factory.CreateFence(device, FenceDescriptor{.m_initialValue = 0u});
   Ptr<Fence> acquireFence = p_factory.CreateFence(device, FenceDescriptor{.m_initialValue = 0u});

   const uint32_t swapchainImageCount = swapchain->GetSwapchainImageCount();
   const auto GetSwapchainIndex = [swapchainImageCount]() -> uint32_t {
      return RenderStateInterface::Get()->GetFrameIndex() % swapchainImageCount;
   };

   struct SubmitCommandBufferContext
   {
      Ptr<CommandBuffer> m_commandBuffer;
      uint64_t m_submitFenceValue = 0u;
   };

   std::queue<SubmitCommandBufferContext> commandBufferContexts;
   std::mutex commandBufferContextsMutex;

   enki::TaskScheduler taskScheduler;
   taskScheduler.Initialize();
   enki::TaskSet renderThread(
       1u,
       [&submitFence, &renderFence, &acquireFence, &commandBufferContexts, &commandBufferContextsMutex, &device, swapchain,
        renderWindow]([[maybe_unused]] enki::TaskSetPartition p_range, [[maybe_unused]] uint32_t p_threadNum)
       {
          uint64_t highestSubmitValue = 0ul;
          auto vkSwapchain = Cast<GHI::Vulkan::Swapchain>(swapchain);

          while (!renderWindow->ShouldClose())
          {
             SubmitCommandBufferContext context;
             {
                std::lock_guard<std::mutex> lock(commandBufferContextsMutex);
                if (commandBufferContexts.empty())
                   continue;
                context = std::move(commandBufferContexts.front());
                commandBufferContexts.pop();
             }

             // Acquire the next swapchain image
             // TODO: see acquire fence note above - binary vs timeline semaphore mismatch
             const uint32_t swapchainIndex = vkSwapchain->AcquireNextImage(acquireFence);
             (void)swapchainIndex;

             // Submit the command buffer, waiting for image acquisition and signalling render done
             {
                std::vector<Ptr<GHI::CommandBuffer>> commandBuffers{context.m_commandBuffer};
                std::vector<FenceSubmitInfo> waitFences{{.m_fence = acquireFence, .m_value = 0u}};
                std::vector<FenceSubmitInfo> signalFences{
                    {.m_fence = renderFence, .m_value = context.m_submitFenceValue},
                    {.m_fence = submitFence, .m_value = context.m_submitFenceValue}};
                device->QueueSubmit(QueueFamilyType::GraphicsQueue, commandBuffers, waitFences, signalFences);
                highestSubmitValue = context.m_submitFenceValue;
             }

             // Present once rendering is complete
             {
                std::vector<Ptr<GHI::Fence>> waitFences{renderFence};
                vkSwapchain->QueuePresent(vkSwapchain, swapchainIndex, waitFences);
             }
          }

          submitFence->WaitForValue(highestSubmitValue);

          while (!commandBufferContexts.empty())
             commandBufferContexts.pop();
       });

   taskScheduler.AddTaskSetToPipe(&renderThread);

   while (!renderWindow->ShouldClose())
   {
      //const uint32_t swapchainIndex = GetSwapchainIndex();
      const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();

      // Stall the CPU until the frame from MaxQueuedFrames ago has finished on the GPU
      const uint64_t waitValue =
          static_cast<uint64_t>(std::max(static_cast<int64_t>(frameIndex) - RendererDefines::MaxQueuedFrames + 1, 0ll));
      submitFence->WaitForValue(waitValue);

      // Record this frame's command buffer
      {
         Ptr<CommandBuffer> commandBuffer =
             p_factory.CreateCommandBuffer(device, CommandBufferDescriptor{.m_queueType = QueueFamilyType::GraphicsQueue});

         // TODO: PipelineBarrierCommand exists in RenderCommands.h but there is no public method on
         //       CommandRecorder / SubCommandRecorder to emit one. When the API is extended, add:
         //         swapchainImage: ImageLayout::Undefined → ImageLayout::ColorAttachment
         //         depthStencilImage: ImageLayout::Undefined → ImageLayout::DepthStencilAttachment

         commandBuffer->SetLineWidth(1.0f);
         commandBuffer->SetDepthBias(0.0f, 0.0f, 0.0f);

         commandBuffer->BindDescriptorPool(descriptorPool);
         commandBuffer->BindPipeline(PipelineBindPoint::Graphics, graphicsPipeline);

         commandBuffer->SetBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});
         commandBuffer->SetDepthBoundsTestEnable(false);
         commandBuffer->SetDepthBounds(0.0f, 0.0f);
         commandBuffer->SetStencilWriteMask(StencilFaceFlags::FrontAndBack, 0u);
         commandBuffer->SetStencilReference(StencilFaceFlags::FrontAndBack, 0u);
         commandBuffer->SetCullMode(CullMode::CullModeNone);
         commandBuffer->SetFrontFace(FrontFace::FrontFaceCounterClockwise);
         commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

         {
            const glm::uvec2 resolution = renderWindow->GetWindowResolution();
            ViewportRect viewport;
            viewport.m_position = {0.0f, 0.0f};
            viewport.m_size = {static_cast<float>(resolution.x), static_cast<float>(resolution.y)};
            viewport.m_minDepth = 0.0f;
            viewport.m_maxDepth = 1.0f;
            std::array<ViewportRect, 1> viewports{viewport};
            commandBuffer->SetViewportWithCount(viewports);
         }

         {
            const glm::uvec2 resolution = renderWindow->GetWindowResolution();
            Rect2D scissor;
            scissor.m_offset = {0, 0};
            scissor.m_extent = {resolution.x, resolution.y};
            std::array<Rect2D, 1> scissors{scissor};
            commandBuffer->SetScissorWithCount(scissors);
         }

         commandBuffer->SetDepthTestEnable(true);
         commandBuffer->SetDepthWriteEnable(true);
         commandBuffer->SetDepthCompareOp(CompareOp::LessOrEqual);
         commandBuffer->SetStencilTestEnable(false);
         commandBuffer->SetStencilOp(StencilFaceFlags::FrontAndBack, StencilOp::Keep, StencilOp::Keep, StencilOp::Keep,
                                     CompareOp::Always);
         commandBuffer->SetRasterizerDiscardEnable(false);
         commandBuffer->SetDepthBiasEnable(false);
         commandBuffer->SetPrimitiveRestartEnable(false);

         // Bind vertex buffer
         {
            BufferViewDescriptor desc;
            desc.m_buffer = vertexBuffer;
            desc.m_format = ResourceFormat::Undefined;
            desc.m_offsetFromBaseAddress = 0u;
            desc.m_bufferViewRange = WholeSize;
            desc.m_usage = BufferUsage::VertexBuffer;
            Ptr<BufferView> view = p_factory.CreateBufferView(device, std::move(desc));

            BindVertexBuffersCommand::VertexBufferView bindView{.m_vertexBufferView = view, .m_stride = sizeof(Vertex)};
            std::array<BindVertexBuffersCommand::VertexBufferView, 1> bindViews{bindView};
            commandBuffer->BindVertexBuffers(0u, bindViews);
         }

         // Bind index buffer
         {
            BufferViewDescriptor desc;
            desc.m_buffer = indexBuffer;
            desc.m_format = ResourceFormat::Undefined;
            desc.m_offsetFromBaseAddress = 0u;
            desc.m_bufferViewRange = WholeSize;
            desc.m_usage = BufferUsage::IndexBuffer;
            Ptr<BufferView> view = p_factory.CreateBufferView(device, std::move(desc));
            commandBuffer->BindIndexBuffer(view, IndexType::Uint32);
         }

         // Begin rendering
         // TODO: Swapchain::InitInternal does not yet wrap native VkImages in GHI Image/ImageView
         //       objects, so GetSwapchainImageViews() returns an empty span. When the swapchain
         //       implementation is completed, replace nullptr with:
         //         swapchain->GetSwapchainImageViews()[swapchainIndex]
         {
            Rect2D renderArea;
            renderArea.m_offset = {0, 0};
            renderArea.m_extent = {swapchainExtend.x, swapchainExtend.y};

            Ptr<ImageView> swapchainImageView = nullptr; // TODO: swapchain->GetSwapchainImageViews()[swapchainIndex]

            RenderingAttachmentInfo colorAttachment;
            colorAttachment.m_imageView = swapchainImageView;
            colorAttachment.m_imageLayout = ImageLayout::ColorAttachment;
            colorAttachment.m_resolveMode = ResolveModeFlags::None;
            colorAttachment.m_resolveImageView = nullptr;
            colorAttachment.m_resolveImageLayout = ImageLayout::Undefined;
            colorAttachment.m_loadOp = AttachmentLoadOp::Clear;
            colorAttachment.m_storeOp = AttachmentStoreOp::Store;
            colorAttachment.m_clearValue = ClearColorValue{.m_clearValFloat = {1.0f, 0.0f, 0.0f, 0.0f}};

            RenderingAttachmentInfo depthStencilAttachment;
            depthStencilAttachment.m_imageView = depthStencilImageView;
            depthStencilAttachment.m_imageLayout = ImageLayout::DepthStencilAttachment;
            depthStencilAttachment.m_resolveMode = ResolveModeFlags::None;
            depthStencilAttachment.m_resolveImageView = nullptr;
            depthStencilAttachment.m_resolveImageLayout = ImageLayout::Undefined;
            depthStencilAttachment.m_loadOp = AttachmentLoadOp::Clear;
            depthStencilAttachment.m_storeOp = AttachmentStoreOp::Store;
            depthStencilAttachment.m_clearValue = ClearColorValue{.m_clearValFloat = {0.0f, 0.0f, 0.0f, 1.0f}};

            std::array<RenderingAttachmentInfo, 1> colorAttachments{colorAttachment};
            commandBuffer->BeginRendering(renderArea, colorAttachments, depthStencilAttachment, depthStencilAttachment);
         }

         commandBuffer->DrawIndexed(3u, 1u, 0u, 0u, 1u);
         commandBuffer->EndRendering();

         // TODO: Transition swapchain image ColorAttachment → PresentSrc once PipelineBarrier is
         //       exposed on CommandRecorder.

         commandBuffer->Compile();

         {
            const uint64_t submitValue = frameIndex + 1u;
            std::lock_guard<std::mutex> lock(commandBufferContextsMutex);
            commandBufferContexts.push(
                SubmitCommandBufferContext{.m_commandBuffer = commandBuffer, .m_submitFenceValue = submitValue});
         }
      }

      RenderStateInterface::Get()->IncrementFrameIndex();
      glfwPollEvents();
   }

   taskScheduler.WaitforAll();
}

int main()
{
   Environment::Create();

   // GLFW must be initialized before VulkanInstance::Get() is first called, because Init() uses
   // glfwGetRequiredInstanceExtensions internally.
   ASSERT(glfwInit(), "Failed to initialize GLFW");
   ASSERT(glfwVulkanSupported(), "Vulkan is not supported");

   // Register the Vulkan ResourceFactory before anything calls ResourceFactory::Get().
   // VulkanInstance::CreatePhysicalDevices() relies on it being registered.
   GHI::Vulkan::ResourceFactory resourceFactory;
   GHI::ResourceFactory::Register(&resourceFactory);

   // Create and register the RendererState
   std::unique_ptr<RenderState> renderState(new RenderState(RenderStateDescriptor{}));
   RenderStateInterface::Register(renderState.get());

   RenderFunction(resourceFactory);

   RenderStateInterface::Unregister();
   renderState = nullptr;

   GHI::ResourceFactory::Unregister();
   glfwTerminate();

   return 0;
}
