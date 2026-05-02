#include <algorithm>
#include <array>
#include <chrono>
#include <thread>
#include <vector>

#include <glm/glm.hpp>

#include <Util/Util.h>
#include <Util/Logger.h>
#include <IO/FileIO.h>
#include <Module/Module.h>

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
#include <GHI/DescriptorSet.h>
#include <GHI/DescriptorSetLayout.h>
#include <GHI/VertexInputState.h>
#include <GHI/RenderCommands.h>
#include <GHI/Device.h>
#include <GHI/PhysicalDevice.h>

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
   std::vector<uint8_t> vertexShaderBin;
   std::vector<uint8_t> fragmentShaderBin;
   {
      using namespace Foundation::IO;

      {
         auto io = FileIO::CreateFileIO(
             FileIODescriptor{.m_path = "Data/Shaders/triangle.vert.spv",
                              .m_fileIOFlags = Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});
         io->Open();
         const uint64_t size = io->GetFileSize();
         vertexShaderBin.resize(size);
         io->Read(vertexShaderBin.data(), size);
      }
      {
         auto io = FileIO::CreateFileIO(
             FileIODescriptor{.m_path = "Data/Shaders/triangle.frag.spv",
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

   Ptr<BufferView> uniformBufferView;
   {
      BufferViewDescriptor desc;
      desc.m_buffer = uniformBuffer;
      desc.m_format = ResourceFormat::Undefined;
      desc.m_offsetFromBaseAddress = 0u;
      desc.m_bufferViewRange = sizeof(Mvp);
      desc.m_usage = BufferUsage::Uniform;
      uniformBufferView = p_factory.CreateBufferView(device, std::move(desc));
   }

   Ptr<DescriptorSetLayout> descriptorSetLayout;
   {
      DescriptorSetLayoutDescriptor desc;
      desc.m_setIndex = 0u;
      desc.m_stages = {
          ShaderStageReflectionSource{.m_shaderModule = vertexShaderModule, .m_stage = ShaderStageFlag::Vertex},
          ShaderStageReflectionSource{.m_shaderModule = fragmentShaderModule, .m_stage = ShaderStageFlag::Fragment}};
      descriptorSetLayout = p_factory.CreateDescriptorSetLayout(device, std::move(desc));
   }

   Ptr<DescriptorPool> descriptorPool = p_factory.CreateDescriptorPool(
       device, DescriptorPoolDescriptor{.m_poolType = DescriptorPoolType::Resource, .m_poolSize = 4096u});

   Ptr<DescriptorSet> descriptorSet;
   {
      DescriptorSetDescriptor desc;
      desc.m_pool = descriptorPool;
      desc.m_layout = descriptorSetLayout;
      descriptorSet = p_factory.CreateDescriptorSet(device, std::move(desc));

      // Compile creates the first immutable descriptor version used by command buffers.
      descriptorSet->BeginWrite()
          .WriteUniformBuffer("ubo", uniformBufferView)
          .Compile();
   }

   // Create the depth/stencil image
   constexpr ResourceFormat depthStencilFormat = ResourceFormat::D32SfloatS8Uint;
   const glm::uvec2 swapchainExtend = swapchain->GetExtend();
   Ptr<Image> depthStencilImage;
   {
      ImageDescriptor desc;
      desc.m_imageUsageFlags = ImageUsageFlags::DepthStencilAttachment;
      desc.m_imageType = ImageType::Image2D;
      desc.m_extend = glm::uvec3(swapchainExtend.x, swapchainExtend.y, 1u);
      desc.m_format = depthStencilFormat;
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
      desc.m_format = ResourceFormat::Invalid;
      desc.m_baseMipLevel = 0u;
      desc.m_mipLevelCount = 1u;
      desc.m_baseArrayLayer = 0u;
      desc.m_arrayLayerCount = 1u;
      desc.m_aspectMask = ImageAspectFlags::Depth | ImageAspectFlags::Stencil;
      depthStencilImageView = p_factory.CreateImageView(device, std::move(desc));
   }

   // Create vertex input state via the factory
   Ptr<GHI::VertexInputState> vertexInputState = p_factory.CreateVertexInputState(device, VertexInputStateDescriptor{});
   {
      GHI::VertexInputBinding& binding = vertexInputState->AddVertexInputBinding(VertexInputRate::VertexInputRateVertex);
      binding.m_stride = sizeof(Vertex);
      binding.AddVertexInputAttribute(0u, ResourceFormat::R32G32B32Sfloat, offsetof(Vertex, position));
      binding.AddVertexInputAttribute(1u, ResourceFormat::R32G32B32Sfloat, offsetof(Vertex, color));
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
      desc.m_descriptorSetLayouts = {descriptorSetLayout};
      desc.m_vertexInputState = vertexInputState;
      desc.m_polygonMode = PolygonMode::PolygonModeFill;
      desc.m_primitiveTopologyClass = PrimitiveTopologyClass::Triangle;
      desc.m_colorBlendAttachmentStates = {colorBlend};
      desc.m_colorAttachmentFormats = {swapchain->GetFormat()};
      desc.m_depthFormat = depthStencilFormat;
      desc.m_stencilFormat = depthStencilFormat;

      graphicsPipeline = p_factory.CreateGraphicsPipeline(device, std::move(desc));
   }

   // Timeline fence for CPU-GPU frame pacing.
   Ptr<Fence> submitFence = p_factory.CreateFence(
       device, FenceDescriptor{.m_type = SemaphoreType::Timeline, .m_initialValue = 0u});

   // Binary semaphores for WSI acquire/present synchronization.
   std::array<Ptr<Fence>, RendererDefines::MaxQueuedFrames> renderFences;
   std::array<Ptr<Fence>, RendererDefines::MaxQueuedFrames> acquireFences;
   for (uint32_t i = 0u; i < RendererDefines::MaxQueuedFrames; i++)
   {
      renderFences[i] = p_factory.CreateFence(device, FenceDescriptor{.m_type = SemaphoreType::Binary});
      acquireFences[i] = p_factory.CreateFence(device, FenceDescriptor{.m_type = SemaphoreType::Binary});
   }

   // TODO: Went back to single threaded approach recording and submitting
   const uint32_t swapchainImageCount = swapchain->GetSwapchainImageCount();
   const uint32_t maxFramesInFlight =
       std::min(RendererDefines::MaxQueuedFrames, std::max(1u, swapchainImageCount - 1u));
   ASSERT(swapchain->GetSwapchainImageViews().size() == swapchainImageCount, "Swapchain image views were not created");
   std::vector<bool> swapchainImageSeen(swapchainImageCount, false);
   bool depthStencilImageSeen = false;

   std::array<Ptr<CommandBuffer>, RendererDefines::MaxQueuedFrames> commandBuffersInFlight;

   const auto PollEventsUntil = [&renderWindow](auto&& p_predicate) {
      while (!p_predicate() && !renderWindow->ShouldClose())
      {
         renderWindow->PollEvents();
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   };

   while (!renderWindow->ShouldClose())
   {
      const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();

      // Stall the CPU so we do not acquire more swapchain images than WSI allows.
      const uint64_t waitValue =
          static_cast<uint64_t>(std::max(static_cast<int64_t>(frameIndex) - maxFramesInFlight + 1, 0ll));
      PollEventsUntil([&submitFence, waitValue]() {
         return submitFence->IsValueSignaled(waitValue);
      });
      if (renderWindow->ShouldClose())
      {
         break;
      }

      const uint32_t syncIndex = static_cast<uint32_t>(frameIndex % maxFramesInFlight);
      commandBuffersInFlight[syncIndex].reset();
      descriptorPool->ProcessDeletionQueue();

      Ptr<Fence> acquireFence = acquireFences[syncIndex];
      uint32_t swapchainIndex = static_cast<uint32_t>(-1);
      PollEventsUntil([&swapchain, &acquireFence, &swapchainIndex]() {
         constexpr uint64_t AcquireTimeoutNanoseconds = 1'000'000u;
         swapchainIndex = swapchain->AcquireNextImage(acquireFence, AcquireTimeoutNanoseconds);
         return swapchainIndex != static_cast<uint32_t>(-1);
      });
      if (renderWindow->ShouldClose())
      {
         break;
      }

      Ptr<ImageView> swapchainImageView = swapchain->GetSwapchainImageViews()[swapchainIndex];
      const ImageLayout swapchainOldLayout =
          swapchainImageSeen[swapchainIndex] ? ImageLayout::PresentSrc : ImageLayout::Undefined;
      const ImageLayout depthStencilOldLayout =
          depthStencilImageSeen ? ImageLayout::DepthStencilAttachment : ImageLayout::Undefined;

      // Record this frame's command buffer
      {
         Ptr<CommandBuffer> commandBuffer =
             p_factory.CreateCommandBuffer(device, CommandBufferDescriptor{.m_queueType = QueueFamilyType::GraphicsQueue});

         constexpr uint32_t IgnoredQueueFamily = static_cast<uint32_t>(-1);
         commandBuffer->PipelineBarrier()
             ->AddImageBarrier(PipelineStageFlags::None, AccessFlags::None, PipelineStageFlags::ColorAttachmentOut,
                               AccessFlags::ColorAttachmentWrite, swapchainOldLayout, ImageLayout::ColorAttachment,
                               IgnoredQueueFamily, IgnoredQueueFamily, swapchainImageView)
             ->AddImageBarrier(depthStencilOldLayout == ImageLayout::Undefined ? PipelineStageFlags::None
                                                                               : PipelineStageFlags::LateFragmentTests,
                               depthStencilOldLayout == ImageLayout::Undefined ? AccessFlags::None
                                                                               : AccessFlags::DepthStencilAttachmentWrite,
                               PipelineStageFlags::EarlyFragmentTests | PipelineStageFlags::LateFragmentTests,
                               AccessFlags::DepthStencilAttachmentWrite, depthStencilOldLayout,
                               ImageLayout::DepthStencilAttachment, IgnoredQueueFamily, IgnoredQueueFamily,
                               depthStencilImageView);


         commandBuffer->SetLineWidth(1.0f);
         commandBuffer->SetDepthBias(0.0f, 0.0f, 0.0f);

         commandBuffer->BindDescriptorPool(descriptorPool);
         commandBuffer->BindPipeline(PipelineBindPoint::Graphics, graphicsPipeline);
         commandBuffer->BindDescriptorSet(descriptorSet, PipelineBindPoint::Graphics, graphicsPipeline);

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
         {
            Rect2D renderArea;
            renderArea.m_offset = {0, 0};
            renderArea.m_extent = {swapchainExtend.x, swapchainExtend.y};

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

         commandBuffer->PipelineBarrier()->AddImageBarrier(
             PipelineStageFlags::ColorAttachmentOut, AccessFlags::ColorAttachmentWrite, PipelineStageFlags::None,
             AccessFlags::None, ImageLayout::ColorAttachment, ImageLayout::PresentSrc, IgnoredQueueFamily,
             IgnoredQueueFamily, swapchainImageView);

         commandBuffer->Compile();
         swapchainImageSeen[swapchainIndex] = true;
         depthStencilImageSeen = true;

         const uint64_t submitValue = frameIndex + 1u;
         commandBuffersInFlight[syncIndex] = commandBuffer;

         Ptr<Fence> renderFence = renderFences[syncIndex];
         std::vector<Ptr<GHI::CommandBuffer>> commandBuffers{commandBuffer};
         std::vector<FenceSubmitInfo> waitFences{{.m_fence = acquireFence, .m_value = 0u}};
         std::vector<FenceSubmitInfo> signalFences{{.m_fence = renderFence, .m_value = 0u},
                                                   {.m_fence = submitFence, .m_value = submitValue}};
         device->QueueSubmit(QueueFamilyType::GraphicsQueue, commandBuffers, waitFences, signalFences);

         std::vector<Ptr<GHI::Fence>> presentWaitFences{renderFence};
         swapchain->QueuePresent(swapchainIndex, presentWaitFences);
      }

      RenderStateInterface::Get()->IncrementFrameIndex();
      renderWindow->PollEvents();
   }

   const uint64_t finalWaitValue = RenderStateInterface::Get()->GetFrameIndex();
   submitFence->WaitForValue(finalWaitValue);
   for (Ptr<CommandBuffer>& commandBuffer : commandBuffersInFlight)
   {
      commandBuffer.reset();
   }
   descriptorPool->ProcessDeletionQueue();
}

int main()
{
   Environment::Create();

   ModuleLoader moduleLoader;
   moduleLoader.LoadModule("GHIVulkan.dll");

   // Bootstrap the platform-specific backend without any Vulkan headers in this translation unit.
   std::unique_ptr<GHI::ResourceFactory> resourceFactory = GHI::CreatePlatformResourceFactory();
   GHI::ResourceFactory::Register(resourceFactory.get());

   // Create and register the RendererState
   std::unique_ptr<RenderState> renderState(new RenderState(RenderStateDescriptor{}));
   RenderStateInterface::Register(renderState.get());

   RenderFunction(*resourceFactory);

   RenderStateInterface::Unregister();
   renderState = nullptr;

   GHI::ResourceFactory::Unregister();

   return 0;
}
