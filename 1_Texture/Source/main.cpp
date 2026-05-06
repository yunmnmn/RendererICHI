#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <thread>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
#include <GHI/Sampler.h>
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
#include <GHI/RenderGraph.h>
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
   float texCoord[2] = {};
};

struct Mvp
{
   glm::mat4 projectionMatrix;
   glm::mat4 modelMatrix;
   glm::mat4 viewMatrix;
};

// clang-format off
std::array<Ptr<Buffer>, 2u> CreateCubeBuffers(GHI::ResourceFactory& p_factory, Ptr<GHI::Device> p_device)
{
   const std::vector<Vertex> vertices = {
      // Front face (z = +0.5)
      {.position = {-0.5f, -0.5f, +0.5f}, .texCoord = {0.0f, 1.0f}},
      {.position = {+0.5f, -0.5f, +0.5f}, .texCoord = {1.0f, 1.0f}},
      {.position = {+0.5f, +0.5f, +0.5f}, .texCoord = {1.0f, 0.0f}},
      {.position = {-0.5f, +0.5f, +0.5f}, .texCoord = {0.0f, 0.0f}},
      // Back face (z = -0.5)
      {.position = {+0.5f, -0.5f, -0.5f}, .texCoord = {0.0f, 1.0f}},
      {.position = {-0.5f, -0.5f, -0.5f}, .texCoord = {1.0f, 1.0f}},
      {.position = {-0.5f, +0.5f, -0.5f}, .texCoord = {1.0f, 0.0f}},
      {.position = {+0.5f, +0.5f, -0.5f}, .texCoord = {0.0f, 0.0f}},
      // Left face (x = -0.5)
      {.position = {-0.5f, -0.5f, -0.5f}, .texCoord = {0.0f, 1.0f}},
      {.position = {-0.5f, -0.5f, +0.5f}, .texCoord = {1.0f, 1.0f}},
      {.position = {-0.5f, +0.5f, +0.5f}, .texCoord = {1.0f, 0.0f}},
      {.position = {-0.5f, +0.5f, -0.5f}, .texCoord = {0.0f, 0.0f}},
      // Right face (x = +0.5)
      {.position = {+0.5f, -0.5f, +0.5f}, .texCoord = {0.0f, 1.0f}},
      {.position = {+0.5f, -0.5f, -0.5f}, .texCoord = {1.0f, 1.0f}},
      {.position = {+0.5f, +0.5f, -0.5f}, .texCoord = {1.0f, 0.0f}},
      {.position = {+0.5f, +0.5f, +0.5f}, .texCoord = {0.0f, 0.0f}},
      // Top face (y = +0.5)
      {.position = {-0.5f, +0.5f, +0.5f}, .texCoord = {0.0f, 1.0f}},
      {.position = {+0.5f, +0.5f, +0.5f}, .texCoord = {1.0f, 1.0f}},
      {.position = {+0.5f, +0.5f, -0.5f}, .texCoord = {1.0f, 0.0f}},
      {.position = {-0.5f, +0.5f, -0.5f}, .texCoord = {0.0f, 0.0f}},
      // Bottom face (y = -0.5)
      {.position = {+0.5f, -0.5f, +0.5f}, .texCoord = {0.0f, 1.0f}},
      {.position = {-0.5f, -0.5f, +0.5f}, .texCoord = {1.0f, 1.0f}},
      {.position = {-0.5f, -0.5f, -0.5f}, .texCoord = {1.0f, 0.0f}},
      {.position = {+0.5f, -0.5f, -0.5f}, .texCoord = {0.0f, 0.0f}},
   };
   // clang-format on

   const std::vector<uint32_t> indices = {
       0,  1,  2,  0,  2,  3,  // front
       4,  5,  6,  4,  6,  7,  // back
       8,  9,  10, 8,  10, 11, // left
       12, 13, 14, 12, 14, 15, // right
       16, 17, 18, 16, 18, 19, // top
       20, 21, 22, 20, 22, 23, // bottom
   };

   const uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);
   const uint32_t indexBufferSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);

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
      desc.m_requestBufferSize = indexBufferSize;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_bufferUsageFlags = BufferUsageFlags::TransferDestination | BufferUsageFlags::IndexBuffer;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      desc.m_initialData = indices.data();
      desc.m_initialDataSize = indexBufferSize;
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

// Generate a checkerboard texture, write to PNG, then load it back with stb_image.
// Returns pixel data (RGBA) + dimensions.
struct TextureData
{
   std::vector<uint8_t> pixels;
   int width = 0;
   int height = 0;
};

TextureData LoadOrCreateTexture()
{
   const char* texturePath = "Data/Textures/checkerboard.png";
   std::filesystem::create_directories("Data/Textures");

   // Generate a colorful checkerboard PNG using stb_image_write
   constexpr int genWidth = 256;
   constexpr int genHeight = 256;
   std::vector<uint8_t> genPixels(genWidth * genHeight * 4);
   for (int y = 0; y < genHeight; y++)
   {
      for (int x = 0; x < genWidth; x++)
      {
         const bool checker = ((x >> 5) ^ (y >> 5)) & 1;
         const int i = (y * genWidth + x) * 4;
         genPixels[i + 0] = checker ? 220u : 40u;
         genPixels[i + 1] = checker ? 200u : 40u;
         genPixels[i + 2] = checker ? 50u : 180u;
         genPixels[i + 3] = 255u;
      }
   }
   stbi_write_png(texturePath, genWidth, genHeight, 4, genPixels.data(), genWidth * 4);

   // Load it back with stb_image
   int width = 0, height = 0, channels = 0;
   stbi_uc* raw = stbi_load(texturePath, &width, &height, &channels, STBI_rgb_alpha);
   ASSERT(raw != nullptr, "Failed to load texture with stb_image");

   TextureData data;
   data.width = width;
   data.height = height;
   data.pixels.assign(raw, raw + width * height * 4);
   stbi_image_free(raw);
   return data;
}

void RenderFunction(GHI::ResourceFactory& p_factory)
{
   std::vector<Ptr<GHI::PhysicalDevice>> physicalDevices = p_factory.GetPhysicalDevices();
   Ptr<GHI::PhysicalDevice> physicalDevice = SelectPhysicalDevice(physicalDevices);

   Ptr<GHI::Device> device = p_factory.CreateDevice(DeviceDescriptor{.m_physicalDevice = physicalDevice});

   Ptr<GHI::RenderWindow> renderWindow = p_factory.CreateRenderWindow(
       device, RenderWindowDescriptor{.m_windowResolution = glm::uvec2(1920u, 1080u), .m_windowTitle = "Texture"});

   Ptr<GHI::Swapchain> swapchain = p_factory.CreateSwapchain(device, SwapchainDescriptor{.m_renderWindow = renderWindow});
   swapchain->Init();

   // Load shaders
   Ptr<GHI::ShaderModule> vertexShaderModule;
   Ptr<GHI::ShaderModule> fragmentShaderModule;
   {
      using namespace Foundation::IO;

      std::vector<uint8_t> vertBin, fragBin;

      {
         auto io = FileIO::CreateFileIO(
             FileIODescriptor{.m_path = "Data/Shaders/texture.vert.spv",
                              .m_fileIOFlags = Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});
         io->Open();
         vertBin.resize(io->GetFileSize());
         io->Read(vertBin.data(), io->GetFileSize());
      }
      {
         auto io = FileIO::CreateFileIO(
             FileIODescriptor{.m_path = "Data/Shaders/texture.frag.spv",
                              .m_fileIOFlags = Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});
         io->Open();
         fragBin.resize(io->GetFileSize());
         io->Read(fragBin.data(), io->GetFileSize());
      }

      vertexShaderModule = p_factory.CreateShaderModule(
          device,
          ShaderModuleDescriptor{.m_spirvBinary = vertBin.data(), .m_binarySizeInBytes = static_cast<uint32_t>(vertBin.size())});
      fragmentShaderModule = p_factory.CreateShaderModule(
          device,
          ShaderModuleDescriptor{.m_spirvBinary = fragBin.data(), .m_binarySizeInBytes = static_cast<uint32_t>(fragBin.size())});
   }

   // Cube geometry
   auto buffers = CreateCubeBuffers(p_factory, device);
   Ptr<Buffer> vertexBuffer = buffers[0];
   Ptr<Buffer> indexBuffer = buffers[1];

   // Uniform buffer (host-visible so we can update it every frame)
   Ptr<Buffer> uniformBuffer;
   void* mappedMvp = nullptr;
   {
      BufferDescriptor desc;
      desc.m_requestBufferSize = sizeof(Mvp);
      desc.m_memoryProperties = MemoryPropertyFlags::HostVisible | MemoryPropertyFlags::HostCoherent;
      desc.m_bufferUsageFlags = BufferUsageFlags::Uniform;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      uniformBuffer = p_factory.CreateBuffer(device, std::move(desc));
      mappedMvp = uniformBuffer->Map(0u);
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

   // Load texture and upload to GPU
   TextureData texData = LoadOrCreateTexture();

   Ptr<Image> textureImage;
   {
      ImageDescriptor desc;
      desc.m_imageUsageFlags = ImageUsageFlags::Sampled;
      desc.m_imageType = ImageType::Image2D;
      desc.m_extend = glm::uvec3(texData.width, texData.height, 1u);
      desc.m_format = ResourceFormat::R8G8B8A8Unorm;
      desc.m_mipLevels = 1u;
      desc.m_arrayLayers = 1u;
      desc.m_imageTiling = ImageTiling::TilingOptimal;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_initialLayout = ImageLayout::Undefined;
      desc.m_initialData = texData.pixels.data();
      desc.m_initialDataSize = static_cast<uint64_t>(texData.pixels.size());
      textureImage = p_factory.CreateImage(device, std::move(desc));
   }

   Ptr<ImageView> textureImageView;
   {
      ImageViewDescriptor desc;
      desc.m_image = textureImage;
      desc.m_extend = textureImage->GetImageExtend();
      desc.m_viewType = ImageViewType::View2D;
      desc.m_format = ResourceFormat::Invalid;
      desc.m_baseMipLevel = 0u;
      desc.m_mipLevelCount = 1u;
      desc.m_baseArrayLayer = 0u;
      desc.m_arrayLayerCount = 1u;
      desc.m_aspectMask = ImageAspectFlags::Color;
      textureImageView = p_factory.CreateImageView(device, std::move(desc));
   }

   // Create sampler
   Ptr<GHI::Sampler> textureSampler =
       p_factory.CreateSampler(device, SamplerDescriptor{.m_magFilter = SamplerFilter::Linear,
                                                         .m_minFilter = SamplerFilter::Linear,
                                                         .m_mipmapMode = SamplerMipmapMode::Linear,
                                                         .m_addressModeU = SamplerAddressMode::Repeat,
                                                         .m_addressModeV = SamplerAddressMode::Repeat,
                                                         .m_addressModeW = SamplerAddressMode::Repeat});

   // Descriptor set layout (reflected from both shaders)
   Ptr<DescriptorSetLayout> descriptorSetLayout;
   {
      DescriptorSetLayoutDescriptor desc;
      desc.m_setIndex = 0u;
      desc.m_stages = {ShaderStageReflectionSource{.m_shaderModule = vertexShaderModule, .m_stage = ShaderStageFlag::Vertex},
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
      descriptorSet->BeginWrite()
          .WriteUniformBuffer("ubo", uniformBufferView)
          .WriteSampledImage("texColor", textureImageView)
          .WriteSampler("texSampler", textureSampler)
          .Compile();
   }

   // Depth/stencil
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

   // Vertex input state
   Ptr<GHI::VertexInputState> vertexInputState = p_factory.CreateVertexInputState(device, VertexInputStateDescriptor{});
   {
      GHI::VertexInputBinding& binding = vertexInputState->AddVertexInputBinding(VertexInputRate::VertexInputRateVertex);
      binding.m_stride = sizeof(Vertex);
      binding.AddVertexInputAttribute(0u, ResourceFormat::R32G32B32Sfloat, offsetof(Vertex, position));
      binding.AddVertexInputAttribute(1u, ResourceFormat::R32G32Sfloat, offsetof(Vertex, texCoord));
   }

   // Graphics pipeline
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

   // Synchronization
   Ptr<Fence> submitFence = p_factory.CreateFence(device, FenceDescriptor{.m_type = SemaphoreType::Timeline, .m_initialValue = 0u});

   std::array<Ptr<Fence>, RendererDefines::MaxQueuedFrames> renderFences;
   std::array<Ptr<Fence>, RendererDefines::MaxQueuedFrames> acquireFences;
   for (uint32_t i = 0u; i < RendererDefines::MaxQueuedFrames; i++)
   {
      renderFences[i] = p_factory.CreateFence(device, FenceDescriptor{.m_type = SemaphoreType::Binary});
      acquireFences[i] = p_factory.CreateFence(device, FenceDescriptor{.m_type = SemaphoreType::Binary});
   }

   const uint32_t swapchainImageCount = swapchain->GetSwapchainImageCount();
   const uint32_t maxFramesInFlight = std::min(RendererDefines::MaxQueuedFrames, std::max(1u, swapchainImageCount - 1u));
   ASSERT(swapchain->GetSwapchainImageViews().size() == swapchainImageCount, "Swapchain image view count mismatch");

   std::vector<bool> swapchainImageSeen(swapchainImageCount, false);
   bool depthStencilImageSeen = false;
   bool textureImageSeen = false;

   std::array<Ptr<CommandBuffer>, RendererDefines::MaxQueuedFrames> commandBuffersInFlight;

   const auto PollEventsUntil = [&renderWindow](auto&& p_predicate) {
      while (!p_predicate() && !renderWindow->ShouldClose())
      {
         renderWindow->PollEvents();
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   };

   const auto startTime = std::chrono::high_resolution_clock::now();

   while (!renderWindow->ShouldClose())
   {
      const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();

      const uint64_t waitValue = static_cast<uint64_t>(std::max(static_cast<int64_t>(frameIndex) - maxFramesInFlight + 1, 0ll));
      PollEventsUntil([&submitFence, waitValue]() { return submitFence->IsValueSignaled(waitValue); });
      if (renderWindow->ShouldClose())
         break;

      const uint32_t syncIndex = static_cast<uint32_t>(frameIndex % maxFramesInFlight);
      commandBuffersInFlight[syncIndex].reset();

      Ptr<Fence> acquireFence = acquireFences[syncIndex];
      uint32_t swapchainIndex = static_cast<uint32_t>(-1);
      PollEventsUntil([&swapchain, &acquireFence, &swapchainIndex]() {
         constexpr uint64_t AcquireTimeoutNanoseconds = 1'000'000u;
         swapchainIndex = swapchain->AcquireNextImage(acquireFence, AcquireTimeoutNanoseconds);
         return swapchainIndex != static_cast<uint32_t>(-1);
      });
      if (renderWindow->ShouldClose())
         break;

      // Update MVP for rotation
      {
         const float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();
         const glm::uvec2 res = renderWindow->GetWindowResolution();
         const float aspect = static_cast<float>(res.x) / static_cast<float>(res.y);

         Mvp mvp;
         mvp.projectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
         mvp.projectionMatrix[1][1] *= -1.0f; // flip Y for Vulkan NDC
         mvp.modelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
         mvp.viewMatrix = glm::lookAt(glm::vec3(0.0f, 1.5f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
         memcpy(mappedMvp, &mvp, sizeof(Mvp));
      }

      Ptr<ImageView> swapchainImageView = swapchain->GetSwapchainImageViews()[swapchainIndex];
      const ResourceUsage swapchainInitialUsage =
          swapchainImageSeen[swapchainIndex] ? ResourceUsage::Present : ResourceUsage::Undefined;
      const ResourceUsage depthStencilInitialUsage =
          depthStencilImageSeen ? ResourceUsage::DepthStencilWrite : ResourceUsage::Undefined;
      // After initial upload, texture is in ShaderRead layout
      const ResourceUsage textureInitialUsage = textureImageSeen ? ResourceUsage::SampledRead : ResourceUsage::SampledRead;

      {
         Ptr<CommandBuffer> commandBuffer =
             p_factory.CreateCommandBuffer(device, CommandBufferDescriptor{.m_queueType = QueueFamilyType::GraphicsQueue});

         RenderGraph graph;
         p_factory.ConfigureRenderGraph(graph, device);

         const RenderGraphResourceHandle swapchainColor =
             graph.ImportImageView("swapchain color", swapchainImageView, swapchainInitialUsage);
         const RenderGraphResourceHandle depthStencil =
             graph.ImportImageView("depth stencil", depthStencilImageView, depthStencilInitialUsage);
         const RenderGraphResourceHandle textureResource = graph.ImportImageView("texture", textureImageView, textureInitialUsage);

         auto [renderedSwapchainColor, renderedDepthStencil] =
             graph.AddPass("cube")
                 .Read("texture", textureResource, ResourceUsage::SampledRead)
                 .Write("swapchain color", swapchainColor, ResourceUsage::ColorAttachmentWrite)
                 .Write("depth stencil", depthStencil, ResourceUsage::DepthStencilWrite)
             .Execute([&](RenderGraphContext& p_context) {
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
                   colorAttachment.m_imageView = p_context.GetImageView(p_context.Output("swapchain color"));
                   colorAttachment.m_imageLayout = ImageLayout::ColorAttachment;
                   colorAttachment.m_resolveMode = ResolveModeFlags::None;
                   colorAttachment.m_resolveImageView = nullptr;
                   colorAttachment.m_resolveImageLayout = ImageLayout::Undefined;
                   colorAttachment.m_loadOp = AttachmentLoadOp::Clear;
                   colorAttachment.m_storeOp = AttachmentStoreOp::Store;
                   colorAttachment.m_clearValue = ClearColorValue{.m_clearValFloat = {0.1f, 0.1f, 0.15f, 1.0f}};

                   RenderingAttachmentInfo depthStencilAttachment;
                   depthStencilAttachment.m_imageView = p_context.GetImageView(p_context.Output("depth stencil"));
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

                commandBuffer->DrawIndexed(36u, 1u, 0u, 0u, 1u);
                commandBuffer->EndRendering();
             });
         (void)renderedDepthStencil;

         graph.AddPass("present")
             .Read("swapchain color", renderedSwapchainColor, ResourceUsage::Present)
             .NeverCull();

         graph.Execute(*commandBuffer);
         commandBuffer->Compile();

         swapchainImageSeen[swapchainIndex] = true;
         depthStencilImageSeen = true;
         textureImageSeen = true;

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

   uniformBuffer->Unmap();

   const uint64_t finalWaitValue = RenderStateInterface::Get()->GetFrameIndex();
   submitFence->WaitForValue(finalWaitValue);
   for (Ptr<CommandBuffer>& commandBuffer : commandBuffersInFlight)
   {
      commandBuffer.reset();
   }
}

int main()
{
   Environment::Create();

   ModuleLoader moduleLoader;
   moduleLoader.LoadModule("GHIVulkan.dll");

   std::unique_ptr<GHI::ResourceFactory> resourceFactory = GHI::CreatePlatformResourceFactory();
   GHI::ResourceFactory::Register(resourceFactory.get());

   std::unique_ptr<RenderState> renderState(new RenderState(RenderStateDescriptor{}));
   RenderStateInterface::Register(renderState.get());

   RenderFunction(*resourceFactory);

   RenderStateInterface::Unregister();
   renderState = nullptr;

   GHI::ResourceFactory::Unregister();

   return 1;
}
