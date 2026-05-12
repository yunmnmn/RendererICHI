#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <thread>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Util/ImGui/ImGuiContext.h>
#include <GHI/Vulkan/Device.h>
#include <imgui.h>

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
#include <GHI/QueryPool.h>
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
constexpr uint32_t MeshletsPerTask = 32u;
constexpr float CameraFieldOfViewYDegrees = 45.0f;
constexpr float CameraNearPlane = 0.1f;
constexpr float CameraFarPlane = 100.0f;

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
   uint32_t meshletCount = 0u;
   std::array<uint32_t, 3> padding = {};
   glm::vec4 cameraWorldPositionScale = {};
   glm::vec4 frustumData = {};
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

struct MeshQueryStats
{
   double renderMilliseconds = 0.0;
   uint64_t verticesProcessed = 0u;
   uint64_t taskShaderInvocations = 0u;
   uint64_t meshShaderInvocations = 0u;
   bool shaderInvocationsValid = false;
   bool valid = false;
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
      const bool supportsTaskShader = any(physicalDevice->GetPhysicalDeviceFeatureFlags(), PhysicalDeviceFeatureFlags::TaskShader);
      if (!physicalDevice->IsViable() || !supportsMeshShader || !supportsTaskShader)
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

   ASSERT(fallback != nullptr, "No viable GPU with VK_EXT_mesh_shader meshShader/taskShader support found");
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

bool IsSphereInsideViewFrustum(const glm::vec3& p_viewCenter, float p_radius, float p_tanHalfFovY, float p_aspect,
                               float p_nearPlane, float p_farPlane)
{
   const float tanHalfFovX = p_tanHalfFovY * p_aspect;
   const glm::vec3 leftPlane = glm::normalize(glm::vec3(1.0f, 0.0f, -tanHalfFovX));
   const glm::vec3 rightPlane = glm::normalize(glm::vec3(-1.0f, 0.0f, -tanHalfFovX));
   const glm::vec3 bottomPlane = glm::normalize(glm::vec3(0.0f, 1.0f, -p_tanHalfFovY));
   const glm::vec3 topPlane = glm::normalize(glm::vec3(0.0f, -1.0f, -p_tanHalfFovY));

   if (glm::dot(leftPlane, p_viewCenter) < -p_radius)
      return false;
   if (glm::dot(rightPlane, p_viewCenter) < -p_radius)
      return false;
   if (glm::dot(bottomPlane, p_viewCenter) < -p_radius)
      return false;
   if (glm::dot(topPlane, p_viewCenter) < -p_radius)
      return false;
   if (-p_viewCenter.z - p_nearPlane < -p_radius)
      return false;
   if (p_viewCenter.z + p_farPlane < -p_radius)
      return false;

   return true;
}

bool IsMeshletBackfacing(const MeshletGpu& p_meshlet, const glm::mat4& p_modelMatrix,
                         const glm::vec3& p_cameraWorldPosition)
{
   const glm::vec3 coneApexWorld =
       glm::vec3(p_modelMatrix * glm::vec4(glm::vec3(p_meshlet.coneApexCutoff), 1.0f));
   const glm::vec3 coneAxisWorldUnnormalized = glm::mat3(p_modelMatrix) * glm::vec3(p_meshlet.coneAxis);
   const float coneCutoff = p_meshlet.coneApexCutoff.w;
   const glm::vec3 apexToCameraUnnormalized = coneApexWorld - p_cameraWorldPosition;
   const float coneAxisLengthSq = glm::dot(coneAxisWorldUnnormalized, coneAxisWorldUnnormalized);
   const float apexToCameraLengthSq = glm::dot(apexToCameraUnnormalized, apexToCameraUnnormalized);

   if (coneCutoff > 1.0f || coneAxisLengthSq < 1e-8f || apexToCameraLengthSq < 1e-8f)
      return false;

   const glm::vec3 coneAxisWorld = coneAxisWorldUnnormalized * glm::inversesqrt(coneAxisLengthSq);
   const glm::vec3 apexToCamera = apexToCameraUnnormalized * glm::inversesqrt(apexToCameraLengthSq);
   return glm::dot(apexToCamera, coneAxisWorld) >= coneCutoff;
}

uint64_t CountVisibleMeshletVertices(const MeshletModel& p_model, const glm::mat4& p_modelMatrix,
                                     const glm::mat4& p_viewMatrix, const glm::vec3& p_cameraWorldPosition,
                                     float p_modelScale, float p_tanHalfFovY, float p_aspect)
{
   uint64_t vertexCount = 0u;
   for (const MeshletGpu& meshlet : p_model.meshlets)
   {
      const float radius = meshlet.centerRadius.w * p_modelScale;
      const glm::vec3 worldCenter = glm::vec3(p_modelMatrix * glm::vec4(glm::vec3(meshlet.centerRadius), 1.0f));
      const glm::vec3 viewCenter = glm::vec3(p_viewMatrix * glm::vec4(worldCenter, 1.0f));
      if (IsSphereInsideViewFrustum(viewCenter, radius, p_tanHalfFovY, p_aspect, CameraNearPlane, CameraFarPlane) &&
          !IsMeshletBackfacing(meshlet, p_modelMatrix, p_cameraWorldPosition))
      {
         vertexCount += meshlet.data.y;
      }
   }
   return vertexCount;
}

void RenderFunction(GHI::ResourceFactory& p_factory)
{
   MeshletModel model = LoadMeshletModel("Data/Mesh/stanford-bunny.meshlets");

   std::vector<Ptr<GHI::PhysicalDevice>> physicalDevices = p_factory.GetPhysicalDevices();
   Ptr<GHI::PhysicalDevice> physicalDevice = SelectPhysicalDevice(physicalDevices);

   Ptr<GHI::Device> device = p_factory.CreateDevice(DeviceDescriptor{.m_physicalDevice = physicalDevice});

   Ptr<GHI::RenderWindow> renderWindow = p_factory.CreateRenderWindow(
       device, RenderWindowDescriptor{.m_windowResolution = glm::uvec2(1920u, 1080u), .m_windowTitle = "Task Shader Meshlet"});

   Ptr<GHI::Swapchain> swapchain = p_factory.CreateSwapchain(device, SwapchainDescriptor{.m_renderWindow = renderWindow});
   swapchain->Init();

   Render::Util::ImGuiContext imguiCtx;
   {
      Render::Util::ImGuiContextDescriptor desc;
      desc.m_window = renderWindow->GetWindowNative();
      desc.m_device = static_cast<GHI::Vulkan::Device*>(device.get());
      desc.m_swapchainColorFormat = swapchain->GetFormat();
      desc.m_imageCount = swapchain->GetSwapchainImageCount();
      imguiCtx.Init(std::move(desc));
   }

   const std::vector<const char*> modelPaths = {
       "Data/Mesh/stanford-bunny.meshlets",
   };
   const std::vector<const char*> modelNames = {
       "Stanford Bunny",
   };
   int currentModelIndex = 0;
   int pendingModelIndex = 0;

   Ptr<GHI::ShaderModule> taskShaderModule;
   Ptr<GHI::ShaderModule> meshShaderModule;
   Ptr<GHI::ShaderModule> fragmentShaderModule;
   {
      std::vector<uint8_t> taskBin = LoadShaderBinary("Data/Shaders/meshlet.task.spv");
      std::vector<uint8_t> meshBin = LoadShaderBinary("Data/Shaders/meshlet.mesh.spv");
      std::vector<uint8_t> fragBin = LoadShaderBinary("Data/Shaders/meshlet.frag.spv");

      taskShaderModule = p_factory.CreateShaderModule(
          device,
          ShaderModuleDescriptor{.m_spirvBinary = taskBin.data(), .m_binarySizeInBytes = static_cast<uint32_t>(taskBin.size())});
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
      desc.m_stages = {ShaderStageReflectionSource{.m_shaderModule = taskShaderModule, .m_stage = ShaderStageFlag::Task},
                       ShaderStageReflectionSource{.m_shaderModule = meshShaderModule, .m_stage = ShaderStageFlag::Mesh},
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
          PipelineShaderStage{.m_shaderModule = taskShaderModule, .m_shaderStageFlag = ShaderStageFlag::Task},
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

   const float timestampPeriodNanoseconds = physicalDevice->GetTimestampPeriodNanoseconds();
   Ptr<QueryPool> timestampQueryPool = p_factory.CreateQueryPool(
       device, QueryPoolDescriptor{.m_type = QueryType::Timestamp, .m_queryCount = maxFramesInFlight * 2u});
   const bool meshShaderQueryStatsSupported =
       any(physicalDevice->GetPhysicalDeviceFeatureFlags(), PhysicalDeviceFeatureFlags::MeshShaderQueries);
   Ptr<QueryPool> meshStatsQueryPool;
   if (meshShaderQueryStatsSupported)
   {
      meshStatsQueryPool = p_factory.CreateQueryPool(
          device,
          QueryPoolDescriptor{.m_type = QueryType::PipelineStatistics,
                              .m_queryCount = maxFramesInFlight,
                              .m_pipelineStatistics = QueryPipelineStatisticFlags::TaskShaderInvocations |
                                                      QueryPipelineStatisticFlags::MeshShaderInvocations});
   }

   std::array<RenderGraphTimestampQuery, RendererDefines::MaxQueuedFrames> timestampQueries;
   std::array<RenderGraphQuery, RendererDefines::MaxQueuedFrames> meshStatsQueries;
   std::array<uint64_t, RendererDefines::MaxQueuedFrames> verticesProcessedInFlight = {};

   std::vector<bool> swapchainImageSeen(swapchainImageCount, false);
   bool depthStencilImageSeen = false;

   std::array<Ptr<CommandBuffer>, RendererDefines::MaxQueuedFrames> commandBuffersInFlight;

   auto lastFrameTime = std::chrono::steady_clock::now();
   float fps = 0.0f;
   MeshQueryStats meshQueryStats;

   const auto PollEventsUntil = [&renderWindow](auto&& p_predicate) {
      while (!p_predicate() && !renderWindow->ShouldClose())
      {
         renderWindow->PollEvents();
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   };

   const auto startTime = std::chrono::high_resolution_clock::now();
   glm::vec3 modelCenter = (model.boundsMin + model.boundsMax) * 0.5f;
   glm::vec3 modelExtent = model.boundsMax - model.boundsMin;
   float modelScale = 2.4f / std::max(std::max(modelExtent.x, modelExtent.y), modelExtent.z);

   while (!renderWindow->ShouldClose())
   {
      const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();

      const uint64_t waitValue = static_cast<uint64_t>(std::max(static_cast<int64_t>(frameIndex) - maxFramesInFlight + 1, 0ll));
      PollEventsUntil([&submitFence, waitValue]() { return submitFence->IsValueSignaled(waitValue); });
      if (renderWindow->ShouldClose())
         break;

      const uint32_t syncIndex = static_cast<uint32_t>(frameIndex % maxFramesInFlight);
      commandBuffersInFlight[syncIndex].reset();

      if (timestampQueries[syncIndex].IsValid())
      {
         const std::optional<QueryReadbackData> timestamps = timestampQueries[syncIndex].Readback();
         if (timestamps.has_value())
         {
            const uint64_t beginTimestamp = timestamps->GetValue(0u);
            const uint64_t endTimestamp = timestamps->GetValue(1u);
            const uint64_t elapsedTicks = endTimestamp >= beginTimestamp ? endTimestamp - beginTimestamp : 0u;
            meshQueryStats.renderMilliseconds =
                static_cast<double>(elapsedTicks) * static_cast<double>(timestampPeriodNanoseconds) / 1'000'000.0;
            meshQueryStats.verticesProcessed = verticesProcessedInFlight[syncIndex];
            meshQueryStats.shaderInvocationsValid = false;
            if (meshStatsQueries[syncIndex].IsValid())
            {
               const std::optional<QueryReadbackData> meshStats = meshStatsQueries[syncIndex].Readback();
               if (meshStats.has_value())
               {
                  meshQueryStats.taskShaderInvocations = meshStats->GetValue(0u);
                  meshQueryStats.meshShaderInvocations = meshStats->GetValue(1u);
                  meshQueryStats.shaderInvocationsValid = true;
               }
            }
            meshQueryStats.valid = true;
         }
      }

      if (pendingModelIndex != currentModelIndex)
      {
         if (frameIndex > 0u)
            submitFence->WaitForValue(frameIndex);
         for (auto& cb : commandBuffersInFlight)
            cb.reset();

         currentModelIndex = pendingModelIndex;
         model = LoadMeshletModel(modelPaths[currentModelIndex]);
         modelCenter = (model.boundsMin + model.boundsMax) * 0.5f;
         modelExtent = model.boundsMax - model.boundsMin;
         modelScale = 2.4f / std::max(std::max(modelExtent.x, modelExtent.y), modelExtent.z);

         meshletBuffer = CreateStorageBuffer(p_factory, device, model.meshlets.data(), model.meshlets.size() * sizeof(MeshletGpu));
         positionBuffer = CreateStorageBuffer(p_factory, device, model.positions.data(), model.positions.size() * sizeof(glm::vec4));
         vertexIndexBuffer = CreateStorageBuffer(p_factory, device, model.vertexIndices.data(), model.vertexIndices.size() * sizeof(uint32_t));
         triangleIndexBuffer = CreateStorageBuffer(p_factory, device, model.triangleIndices.data(), model.triangleIndices.size() * sizeof(uint32_t));

         descriptorSet->BeginWrite()
             .WriteUniformBuffer("scene", sceneBufferView)
             .WriteStorageBuffer("meshlets", meshletBuffer.view)
             .WriteStorageBuffer("positions", positionBuffer.view)
             .WriteStorageBuffer("vertexIndices", vertexIndexBuffer.view)
             .WriteStorageBuffer("triangleIndices", triangleIndexBuffer.view)
             .Compile();
      }

      {
         const auto now = std::chrono::steady_clock::now();
         const float dt = std::chrono::duration<float>(now - lastFrameTime).count();
         lastFrameTime = now;
         fps = dt > 0.0f ? 1.0f / dt : 0.0f;
      }

      imguiCtx.NewFrame();
      ImGui::Begin("Scene");
      ImGui::Text("FPS: %.1f", fps);
      if (meshQueryStats.valid)
      {
         ImGui::Text("Mesh render: %.3f ms", meshQueryStats.renderMilliseconds);
         ImGui::Text("Verts processed: %llu",
                     static_cast<unsigned long long>(meshQueryStats.verticesProcessed));
         if (meshQueryStats.shaderInvocationsValid)
         {
            ImGui::Text("Task shader invocations: %llu",
                        static_cast<unsigned long long>(meshQueryStats.taskShaderInvocations));
            ImGui::Text("Mesh shader invocations: %llu",
                        static_cast<unsigned long long>(meshQueryStats.meshShaderInvocations));
         }
      }
      else
      {
         ImGui::Text("Mesh render: waiting for query");
      }
      ImGui::Separator();
      if (ImGui::BeginCombo("Model", modelNames[pendingModelIndex]))
      {
         for (int i = 0; i < static_cast<int>(modelNames.size()); i++)
         {
            const bool selected = (i == pendingModelIndex);
            if (ImGui::Selectable(modelNames[i], selected))
               pendingModelIndex = i;
            if (selected)
               ImGui::SetItemDefaultFocus();
         }
         ImGui::EndCombo();
      }
      ImGui::End();

      Ptr<Fence> acquireFence = acquireFences[syncIndex];
      uint32_t swapchainIndex = static_cast<uint32_t>(-1);
      PollEventsUntil([&swapchain, &acquireFence, &swapchainIndex]() {
         constexpr uint64_t AcquireTimeoutNanoseconds = 1'000'000u;
         swapchainIndex = swapchain->AcquireNextImage(acquireFence, AcquireTimeoutNanoseconds);
         return swapchainIndex != static_cast<uint32_t>(-1);
      });
      if (renderWindow->ShouldClose())
         break;

      uint64_t currentFrameVerticesProcessed = 0u;
      {
         const float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();
         const glm::uvec2 res = renderWindow->GetWindowResolution();
         const float aspect = static_cast<float>(res.x) / static_cast<float>(res.y);
         const float fieldOfViewY = glm::radians(CameraFieldOfViewYDegrees);
         const float tanHalfFovY = std::tan(fieldOfViewY * 0.5f);
         const glm::vec3 cameraWorldPosition = glm::vec3(0.0f, 0.6f, 4.2f);

         SceneConstants scene;
         glm::mat4 projectionMatrix = glm::perspective(fieldOfViewY, aspect, CameraNearPlane, CameraFarPlane);
         projectionMatrix[1][1] *= -1.0f;
         const glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                       glm::scale(glm::mat4(1.0f), glm::vec3(modelScale)) *
                                       glm::translate(glm::mat4(1.0f), -modelCenter);
         const glm::mat4 viewMatrix = glm::lookAt(cameraWorldPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

         scene.projectionMatrix = projectionMatrix;
         scene.modelMatrix = modelMatrix;
         scene.viewMatrix = viewMatrix;
         scene.meshletCount = model.header.meshletCount;
         scene.cameraWorldPositionScale = glm::vec4(cameraWorldPosition, modelScale);
         scene.frustumData = glm::vec4(tanHalfFovY, aspect, CameraNearPlane, CameraFarPlane);
         std::memcpy(mappedScene, &scene, sizeof(SceneConstants));

         currentFrameVerticesProcessed = CountVisibleMeshletVertices(model, modelMatrix, viewMatrix, cameraWorldPosition,
                                                                     modelScale, tanHalfFovY, aspect);
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

         const uint32_t timestampQueryIndex = syncIndex * 2u;
         auto [renderedSwapchainColor, renderedDepthStencil, meshPassTiming, meshStatsQuery] =
             graph.AddPass("task shader meshlet bunny")
                 .Write("swapchain color", swapchainColor, ResourceUsage::ColorAttachmentWrite)
                 .Write("depth stencil", depthStencil, ResourceUsage::DepthStencilWrite)
                 .Prepare([](RenderGraphPrepareContext& p_context) {
                    p_context.ClearAttachment("swapchain color",
                                              ClearColorValue{.m_clearValFloat = {0.025f, 0.03f, 0.04f, 1.0f}});
                    p_context.ClearAttachment("depth stencil",
                                              ClearColorValue{.m_clearValFloat = {1.0f, 0.0f, 0.0f, 0.0f}});
                 })
                 .WriteTimestamps(timestampQueryPool, timestampQueryIndex, timestampQueryIndex + 1u)
                 .WriteQuery(meshStatsQueryPool, syncIndex)
                 .Execute([&](RenderGraphContext& p_context) {
            SubCommandRecorder& recorder = p_context.GetRecorder();
            recorder.SetLineWidth(1.0f);
            recorder.SetDepthBias(0.0f, 0.0f, 0.0f);

            recorder.BindDescriptorPool(descriptorPool);
            recorder.BindPipeline(PipelineBindPoint::Graphics, graphicsPipeline);
            recorder.BindDescriptorSet(descriptorSet, PipelineBindPoint::Graphics, graphicsPipeline);

            recorder.SetBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});
            recorder.SetDepthBoundsTestEnable(false);
            recorder.SetDepthBounds(0.0f, 0.0f);
            recorder.SetStencilWriteMask(StencilFaceFlags::FrontAndBack, 0u);
            recorder.SetStencilReference(StencilFaceFlags::FrontAndBack, 0u);
            recorder.SetCullMode(CullMode::CullModeNone);
            recorder.SetFrontFace(FrontFace::FrontFaceCounterClockwise);
            recorder.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

            {
               const glm::uvec2 resolution = renderWindow->GetWindowResolution();
               ViewportRect viewport;
               viewport.m_position = {0.0f, 0.0f};
               viewport.m_size = {static_cast<float>(resolution.x), static_cast<float>(resolution.y)};
               viewport.m_minDepth = 0.0f;
               viewport.m_maxDepth = 1.0f;
               std::array<ViewportRect, 1> viewports{viewport};
               recorder.SetViewportWithCount(viewports);
            }

            {
               const glm::uvec2 resolution = renderWindow->GetWindowResolution();
               Rect2D scissor;
               scissor.m_offset = {0, 0};
               scissor.m_extent = {resolution.x, resolution.y};
               std::array<Rect2D, 1> scissors{scissor};
               recorder.SetScissorWithCount(scissors);
            }

            recorder.SetDepthTestEnable(true);
            recorder.SetDepthWriteEnable(true);
            recorder.SetDepthCompareOp(CompareOp::LessOrEqual);
            recorder.SetStencilTestEnable(false);
            recorder.SetStencilOp(StencilFaceFlags::FrontAndBack, StencilOp::Keep, StencilOp::Keep, StencilOp::Keep,
                                  CompareOp::Always);
            recorder.SetRasterizerDiscardEnable(false);
            recorder.SetDepthBiasEnable(false);
            recorder.SetPrimitiveRestartEnable(false);

            const uint32_t taskGroupCount = (model.header.meshletCount + MeshletsPerTask - 1u) / MeshletsPerTask;
            recorder.DrawMeshTasks(taskGroupCount, 1u, 1u);
         });
         timestampQueries[syncIndex] = meshPassTiming;
         meshStatsQueries[syncIndex] = meshStatsQuery;
         (void)renderedDepthStencil;

         auto [imguiOutput] =
             graph.AddPass("imgui")
                 .ReadWrite("swapchain color", renderedSwapchainColor, ResourceUsage::ColorAttachmentReadWrite)
                 .Execute([&](RenderGraphContext& p_context) {
                    imguiCtx.Render(&p_context.GetRecorder());
                 });

         graph.AddPass("present").Read("swapchain color", imguiOutput, ResourceUsage::Present).NeverCull();

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
         verticesProcessedInFlight[syncIndex] = currentFrameVerticesProcessed;

         std::vector<Ptr<GHI::Fence>> presentWaitFences{renderFence};
         swapchain->QueuePresent(swapchainIndex, presentWaitFences);
      }

      RenderStateInterface::Get()->IncrementFrameIndex();
      renderWindow->PollEvents();
   }

   imguiCtx.Shutdown();

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
