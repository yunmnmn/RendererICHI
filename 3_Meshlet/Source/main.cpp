#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <thread>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Util/Assert.h>
#include <Util/Logger.h>
#include <Util/Util.h>
#include <Module/Module.h>

#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/CommandBuffer.h>
#include <GHI/CommandRecorder.h>
#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSet.h>
#include <GHI/DescriptorSetLayout.h>
#include <GHI/Device.h>
#include <GHI/Fence.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/PhysicalDevice.h>
#include <GHI/RenderCommands.h>
#include <GHI/RenderGraph.h>
#include <GHI/RenderWindow.h>
#include <GHI/Renderer.h>
#include <GHI/RendererTypes.h>
#include <GHI/ResourceFactory.h>
#include <GHI/ShaderModule.h>
#include <GHI/Swapchain.h>

#include <RendererState.h>
#include <RendererStateInterface.h>

using namespace Foundation;
using namespace Render;
using namespace Render::GHI;

namespace
{

constexpr uint32_t MeshletFileMagic = 0x544C434Du;
constexpr uint32_t MeshletFileVersion = 1u;

#pragma pack(push, 1)
struct MeshletFileHeader
{
   uint32_t magic = 0u;
   uint32_t version = 0u;
   uint32_t meshletCount = 0u;
   uint32_t vertexCount = 0u;
   uint32_t triangleCount = 0u;
   uint32_t vertexIndexCount = 0u;
   uint32_t triangleIndexCount = 0u;
};

struct MeshletFileDesc
{
   uint32_t vertexOffset = 0u;
   uint32_t vertexCount = 0u;
   uint32_t triangleOffset = 0u;
   uint32_t triangleCount = 0u;
   float center[3] = {};
   float radius = 0.0f;
   float coneApex[3] = {};
   float coneAxis[3] = {};
   float coneCutoff = 0.0f;
};
#pragma pack(pop)

struct MeshletGpu
{
   glm::uvec4 data = {}; // x: vertex offset, y: vertex count, z: triangle offset, w: triangle count
   glm::vec4 centerRadius = {};
   glm::vec4 coneApexCutoff = {};
   glm::vec4 coneAxis = {};
};

struct SceneConstants
{
   glm::mat4 projectionMatrix = glm::mat4(1.0f);
   glm::mat4 modelMatrix = glm::mat4(1.0f);
   glm::mat4 viewMatrix = glm::mat4(1.0f);
};

struct MeshletModel
{
   MeshletFileHeader header = {};
   std::vector<MeshletGpu> meshlets;
   std::vector<glm::vec4> positions;
   std::vector<uint32_t> vertexIndices;
   std::vector<uint32_t> triangleIndices;
   glm::vec3 boundsMin = glm::vec3(0.0f);
   glm::vec3 boundsMax = glm::vec3(0.0f);
};

struct StorageBufferResource
{
   Ptr<Buffer> buffer;
   Ptr<BufferView> view;
};

static_assert(sizeof(MeshletFileHeader) == 28u);
static_assert(sizeof(MeshletFileDesc) == 60u);
static_assert(sizeof(MeshletGpu) == 64u);

std::vector<uint8_t> ReadBinaryFile(const std::filesystem::path& p_path)
{
   std::ifstream file(p_path, std::ios::binary | std::ios::ate);
   ASSERT(file.is_open(), "Failed to open binary file");

   const std::streamsize size = file.tellg();
   ASSERT(size > 0, "Binary file is empty");
   file.seekg(0, std::ios::beg);

   std::vector<uint8_t> data(static_cast<size_t>(size));
   file.read(reinterpret_cast<char*>(data.data()), size);
   ASSERT(file.good(), "Failed to read binary file");
   return data;
}

std::vector<uint8_t> LoadShaderBinary(const char* p_path)
{
   return ReadBinaryFile(p_path);
}

template <typename T>
const T* ReadArrayAt(const std::vector<uint8_t>& p_data, size_t p_offset, size_t p_count)
{
   const size_t byteCount = sizeof(T) * p_count;
   ASSERT(p_offset <= p_data.size() && byteCount <= p_data.size() - p_offset, "Meshlet file section is out of bounds");
   return reinterpret_cast<const T*>(p_data.data() + p_offset);
}

MeshletModel LoadMeshletModel(const std::filesystem::path& p_path)
{
   const std::vector<uint8_t> data = ReadBinaryFile(p_path);
   ASSERT(data.size() >= sizeof(MeshletFileHeader), "Meshlet file is too small");

   MeshletModel model;
   std::memcpy(&model.header, data.data(), sizeof(MeshletFileHeader));
   ASSERT(model.header.magic == MeshletFileMagic, "Invalid meshlet file magic");
   ASSERT(model.header.version == MeshletFileVersion, "Unsupported meshlet file version");
   ASSERT(model.header.meshletCount > 0u, "Meshlet file has no meshlets");
   ASSERT(model.header.vertexCount > 0u, "Meshlet file has no vertices");
   ASSERT(model.header.triangleCount > 0u, "Meshlet file has no triangles");

   size_t offset = sizeof(MeshletFileHeader);
   const MeshletFileDesc* fileMeshlets = ReadArrayAt<MeshletFileDesc>(data, offset, model.header.meshletCount);
   offset += sizeof(MeshletFileDesc) * static_cast<size_t>(model.header.meshletCount);

   const float* filePositions = ReadArrayAt<float>(data, offset, static_cast<size_t>(model.header.vertexCount) * 3u);
   offset += sizeof(float) * static_cast<size_t>(model.header.vertexCount) * 3u;

   const uint32_t* fileVertexIndices = ReadArrayAt<uint32_t>(data, offset, model.header.vertexIndexCount);
   offset += sizeof(uint32_t) * static_cast<size_t>(model.header.vertexIndexCount);

   const uint8_t* fileTriangleIndices = ReadArrayAt<uint8_t>(data, offset, model.header.triangleIndexCount);
   offset += sizeof(uint8_t) * static_cast<size_t>(model.header.triangleIndexCount);
   ASSERT(offset == data.size(), "Meshlet file has unexpected trailing data");

   model.positions.resize(model.header.vertexCount);
   model.boundsMin = glm::vec3(std::numeric_limits<float>::max());
   model.boundsMax = glm::vec3(std::numeric_limits<float>::lowest());
   for (uint32_t i = 0u; i < model.header.vertexCount; ++i)
   {
      const glm::vec3 position(filePositions[i * 3u + 0u], filePositions[i * 3u + 1u], filePositions[i * 3u + 2u]);
      model.positions[i] = glm::vec4(position, 1.0f);
      model.boundsMin = glm::min(model.boundsMin, position);
      model.boundsMax = glm::max(model.boundsMax, position);
   }

   model.vertexIndices.assign(fileVertexIndices, fileVertexIndices + model.header.vertexIndexCount);
   model.meshlets.resize(model.header.meshletCount);
   model.triangleIndices.reserve(static_cast<size_t>(model.header.triangleCount) * 3u);

   for (uint32_t i = 0u; i < model.header.meshletCount; ++i)
   {
      const MeshletFileDesc& src = fileMeshlets[i];
      ASSERT(src.vertexCount <= 64u, "Meshlet exceeds the shader vertex output limit");
      ASSERT(src.triangleCount <= 124u, "Meshlet exceeds the shader primitive output limit");
      ASSERT(src.vertexOffset + src.vertexCount <= model.header.vertexIndexCount, "Meshlet vertex range is out of bounds");
      ASSERT(src.triangleOffset + src.triangleCount * 3u <= model.header.triangleIndexCount,
             "Meshlet triangle range is out of bounds");

      MeshletGpu& dst = model.meshlets[i];
      dst.data.x = src.vertexOffset;
      dst.data.y = src.vertexCount;
      dst.data.z = static_cast<uint32_t>(model.triangleIndices.size());
      dst.data.w = src.triangleCount;
      dst.centerRadius = glm::vec4(src.center[0], src.center[1], src.center[2], src.radius);
      dst.coneApexCutoff = glm::vec4(src.coneApex[0], src.coneApex[1], src.coneApex[2], src.coneCutoff);
      dst.coneAxis = glm::vec4(src.coneAxis[0], src.coneAxis[1], src.coneAxis[2], 0.0f);

      for (uint32_t triangle = 0u; triangle < src.triangleCount; ++triangle)
      {
         const uint32_t triOffset = src.triangleOffset + triangle * 3u;
         model.triangleIndices.push_back(static_cast<uint32_t>(fileTriangleIndices[triOffset + 0u]));
         model.triangleIndices.push_back(static_cast<uint32_t>(fileTriangleIndices[triOffset + 1u]));
         model.triangleIndices.push_back(static_cast<uint32_t>(fileTriangleIndices[triOffset + 2u]));
      }
   }

   ASSERT(model.triangleIndices.size() == static_cast<size_t>(model.header.triangleCount) * 3u,
          "Converted meshlet triangle index count mismatch");
   return model;
}

Ptr<GHI::PhysicalDevice> SelectPhysicalDevice(const std::vector<Ptr<GHI::PhysicalDevice>>& p_physicalDevices)
{
   Ptr<GHI::PhysicalDevice> fallback;
   for (const Ptr<GHI::PhysicalDevice>& physicalDevice : p_physicalDevices)
   {
      const bool supportsMeshShader = any(physicalDevice->GetPhysicalDeviceFeatureFlags(), PhysicalDeviceFeatureFlags::MeshShader);
      if (!physicalDevice->IsViable() || !supportsMeshShader)
      {
         continue;
      }

      if (physicalDevice->GetGPUTypes() == GPUType::Discrete)
      {
         return physicalDevice;
      }

      if (!fallback)
      {
         fallback = physicalDevice;
      }
   }

   ASSERT(fallback != nullptr, "No viable GPU with VK_EXT_mesh_shader support found");
   return fallback;
}

StorageBufferResource CreateStorageBuffer(GHI::ResourceFactory& p_factory, Ptr<GHI::Device> p_device, const void* p_data,
                                          uint64_t p_sizeInBytes)
{
   StorageBufferResource resource;
   {
      BufferDescriptor desc;
      desc.m_requestBufferSize = p_sizeInBytes;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_bufferUsageFlags = BufferUsageFlags::TransferDestination | BufferUsageFlags::Storage;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      desc.m_initialData = p_data;
      desc.m_initialDataSize = p_sizeInBytes;
      resource.buffer = p_factory.CreateBuffer(p_device, std::move(desc));
   }

   {
      BufferViewDescriptor desc;
      desc.m_buffer = resource.buffer;
      desc.m_format = ResourceFormat::Undefined;
      desc.m_offsetFromBaseAddress = 0u;
      desc.m_bufferViewRange = p_sizeInBytes;
      desc.m_usage = BufferUsage::Storage;
      resource.view = p_factory.CreateBufferView(p_device, std::move(desc));
   }

   return resource;
}

void RenderFunction(GHI::ResourceFactory& p_factory)
{
   MeshletModel model = LoadMeshletModel("Data/Mesh/stanford-bunny.meshlets");

   std::vector<Ptr<GHI::PhysicalDevice>> physicalDevices = p_factory.GetPhysicalDevices();
   Ptr<GHI::PhysicalDevice> physicalDevice = SelectPhysicalDevice(physicalDevices);

   Ptr<GHI::Device> device = p_factory.CreateDevice(DeviceDescriptor{.m_physicalDevice = physicalDevice});

   Ptr<GHI::RenderWindow> renderWindow = p_factory.CreateRenderWindow(
       device, RenderWindowDescriptor{.m_windowResolution = glm::uvec2(1920u, 1080u), .m_windowTitle = "Meshlet"});

   Ptr<GHI::Swapchain> swapchain = p_factory.CreateSwapchain(device, SwapchainDescriptor{.m_renderWindow = renderWindow});
   swapchain->Init();

   Ptr<GHI::ShaderModule> meshShaderModule;
   Ptr<GHI::ShaderModule> fragmentShaderModule;
   {
      std::vector<uint8_t> meshBin = LoadShaderBinary("Data/Shaders/meshlet.mesh.spv");
      std::vector<uint8_t> fragBin = LoadShaderBinary("Data/Shaders/meshlet.frag.spv");

      meshShaderModule = p_factory.CreateShaderModule(
          device,
          ShaderModuleDescriptor{.m_spirvBinary = meshBin.data(), .m_binarySizeInBytes = static_cast<uint32_t>(meshBin.size())});
      fragmentShaderModule = p_factory.CreateShaderModule(
          device,
          ShaderModuleDescriptor{.m_spirvBinary = fragBin.data(), .m_binarySizeInBytes = static_cast<uint32_t>(fragBin.size())});
   }

   Ptr<Buffer> sceneBuffer;
   void* mappedScene = nullptr;
   {
      BufferDescriptor desc;
      desc.m_requestBufferSize = sizeof(SceneConstants);
      desc.m_memoryProperties = MemoryPropertyFlags::HostVisible | MemoryPropertyFlags::HostCoherent;
      desc.m_bufferUsageFlags = BufferUsageFlags::Uniform;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      sceneBuffer = p_factory.CreateBuffer(device, std::move(desc));
      mappedScene = sceneBuffer->Map(0u);
   }

   Ptr<BufferView> sceneBufferView;
   {
      BufferViewDescriptor desc;
      desc.m_buffer = sceneBuffer;
      desc.m_format = ResourceFormat::Undefined;
      desc.m_offsetFromBaseAddress = 0u;
      desc.m_bufferViewRange = sizeof(SceneConstants);
      desc.m_usage = BufferUsage::Uniform;
      sceneBufferView = p_factory.CreateBufferView(device, std::move(desc));
   }

   StorageBufferResource meshletBuffer =
       CreateStorageBuffer(p_factory, device, model.meshlets.data(), model.meshlets.size() * sizeof(MeshletGpu));
   StorageBufferResource positionBuffer =
       CreateStorageBuffer(p_factory, device, model.positions.data(), model.positions.size() * sizeof(glm::vec4));
   StorageBufferResource vertexIndexBuffer =
       CreateStorageBuffer(p_factory, device, model.vertexIndices.data(), model.vertexIndices.size() * sizeof(uint32_t));
   StorageBufferResource triangleIndexBuffer =
       CreateStorageBuffer(p_factory, device, model.triangleIndices.data(), model.triangleIndices.size() * sizeof(uint32_t));

   Ptr<DescriptorSetLayout> descriptorSetLayout;
   {
      DescriptorSetLayoutDescriptor desc;
      desc.m_setIndex = 0u;
      desc.m_stages = {ShaderStageReflectionSource{.m_shaderModule = meshShaderModule, .m_stage = ShaderStageFlag::Mesh},
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
          .WriteUniformBuffer("scene", sceneBufferView)
          .WriteStorageBuffer("meshlets", meshletBuffer.view)
          .WriteStorageBuffer("positions", positionBuffer.view)
          .WriteStorageBuffer("vertexIndices", vertexIndexBuffer.view)
          .WriteStorageBuffer("triangleIndices", triangleIndexBuffer.view)
          .Compile();
   }

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
          PipelineShaderStage{.m_shaderModule = meshShaderModule, .m_shaderStageFlag = ShaderStageFlag::Mesh},
          PipelineShaderStage{.m_shaderModule = fragmentShaderModule, .m_shaderStageFlag = ShaderStageFlag::Fragment}};
      desc.m_descriptorSetLayouts = {descriptorSetLayout};
      desc.m_vertexInputState = nullptr;
      desc.m_polygonMode = PolygonMode::PolygonModeFill;
      desc.m_primitiveTopologyClass = PrimitiveTopologyClass::Triangle;
      desc.m_colorBlendAttachmentStates = {colorBlend};
      desc.m_colorAttachmentFormats = {swapchain->GetFormat()};
      desc.m_depthFormat = depthStencilFormat;
      desc.m_stencilFormat = depthStencilFormat;
      graphicsPipeline = p_factory.CreateGraphicsPipeline(device, std::move(desc));
   }

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

   std::array<Ptr<CommandBuffer>, RendererDefines::MaxQueuedFrames> commandBuffersInFlight;

   const auto PollEventsUntil = [&renderWindow](auto&& p_predicate) {
      while (!p_predicate() && !renderWindow->ShouldClose())
      {
         renderWindow->PollEvents();
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   };

   const auto startTime = std::chrono::high_resolution_clock::now();
   const glm::vec3 modelCenter = (model.boundsMin + model.boundsMax) * 0.5f;
   const glm::vec3 modelExtent = model.boundsMax - model.boundsMin;
   const float modelScale = 2.4f / std::max(std::max(modelExtent.x, modelExtent.y), modelExtent.z);

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

      {
         const float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();
         const glm::uvec2 res = renderWindow->GetWindowResolution();
         const float aspect = static_cast<float>(res.x) / static_cast<float>(res.y);

         SceneConstants scene;
         glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
         projectionMatrix[1][1] *= -1.0f;
         const glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                       glm::scale(glm::mat4(1.0f), glm::vec3(modelScale)) *
                                       glm::translate(glm::mat4(1.0f), -modelCenter);
         const glm::mat4 viewMatrix =
             glm::lookAt(glm::vec3(0.0f, 0.6f, 4.2f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

         scene.projectionMatrix = projectionMatrix;
         scene.modelMatrix = modelMatrix;
         scene.viewMatrix = viewMatrix;
         std::memcpy(mappedScene, &scene, sizeof(SceneConstants));
      }

      Ptr<ImageView> swapchainImageView = swapchain->GetSwapchainImageViews()[swapchainIndex];
      const ResourceUsage swapchainInitialUsage =
          swapchainImageSeen[swapchainIndex] ? ResourceUsage::Present : ResourceUsage::Undefined;
      const ResourceUsage depthStencilInitialUsage =
          depthStencilImageSeen ? ResourceUsage::DepthStencilWrite : ResourceUsage::Undefined;

      {
         Ptr<CommandBuffer> commandBuffer =
             p_factory.CreateCommandBuffer(device, CommandBufferDescriptor{.m_queueType = QueueFamilyType::GraphicsQueue});

         RenderGraph graph;
         p_factory.ConfigureRenderGraph(graph, device);

         const RenderGraphResourceHandle swapchainColor =
             graph.ImportImageView("swapchain color", swapchainImageView, swapchainInitialUsage);
         const RenderGraphResourceHandle depthStencil =
             graph.ImportImageView("depth stencil", depthStencilImageView, depthStencilInitialUsage);

         auto [renderedSwapchainColor, renderedDepthStencil] =
             graph.AddPass("meshlet bunny")
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
                       colorAttachment.m_clearValue = ClearColorValue{.m_clearValFloat = {0.025f, 0.03f, 0.04f, 1.0f}};

                       RenderingAttachmentInfo depthStencilAttachment;
                       depthStencilAttachment.m_imageView = p_context.GetImageView(p_context.Output("depth stencil"));
                       depthStencilAttachment.m_imageLayout = ImageLayout::DepthStencilAttachment;
                       depthStencilAttachment.m_resolveMode = ResolveModeFlags::None;
                       depthStencilAttachment.m_resolveImageView = nullptr;
                       depthStencilAttachment.m_resolveImageLayout = ImageLayout::Undefined;
                       depthStencilAttachment.m_loadOp = AttachmentLoadOp::Clear;
                       depthStencilAttachment.m_storeOp = AttachmentStoreOp::Store;
                       depthStencilAttachment.m_clearValue = ClearColorValue{.m_clearValFloat = {1.0f, 0.0f, 0.0f, 0.0f}};

                       std::array<RenderingAttachmentInfo, 1> colorAttachments{colorAttachment};
                       commandBuffer->BeginRendering(renderArea, colorAttachments, depthStencilAttachment, depthStencilAttachment);
                    }

                    commandBuffer->DrawMeshTasks(model.header.meshletCount, 1u, 1u);
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

   sceneBuffer->Unmap();

   const uint64_t finalWaitValue = RenderStateInterface::Get()->GetFrameIndex();
   submitFence->WaitForValue(finalWaitValue);
   for (Ptr<CommandBuffer>& commandBuffer : commandBuffersInFlight)
   {
      commandBuffer.reset();
   }
}

} // namespace

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
