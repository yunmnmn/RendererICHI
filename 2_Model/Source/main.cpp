#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
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
   float normal[3] = {};
};

struct Mvp
{
   glm::mat4 projectionMatrix;
   glm::mat4 modelMatrix;
   glm::mat4 viewMatrix;
};

struct TextureData
{
   std::vector<uint8_t> pixels;
   int width = 0;
   int height = 0;
};

struct ModelData
{
   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   std::filesystem::path texturePath;
};

struct ObjVertexRef
{
   int positionIndex = 0;
   int texCoordIndex = 0;
   int normalIndex = 0;
};

uint8_t ToByte(float p_value)
{
   const float clamped = std::clamp(p_value, 0.0f, 255.0f);
   return static_cast<uint8_t>(clamped);
}

void EnsureCornellBoxTexture(const std::filesystem::path& p_texturePath)
{
   if (std::filesystem::exists(p_texturePath))
   {
      return;
   }

   std::filesystem::create_directories(p_texturePath.parent_path());

   constexpr int width = 512;
   constexpr int height = 512;
   std::vector<uint8_t> pixels(static_cast<size_t>(width * height * 4));

   for (int y = 0; y < height; y++)
   {
      for (int x = 0; x < width; x++)
      {
         const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(width);
         const float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(height);

         glm::vec3 color(218.0f, 214.0f, 196.0f);
         if (u >= 0.5f && v < 0.5f)
         {
            color = glm::vec3(178.0f, 48.0f, 38.0f);
         }
         else if (u < 0.5f && v >= 0.5f)
         {
            color = glm::vec3(58.0f, 142.0f, 84.0f);
         }
         else if (u >= 0.5f && v >= 0.5f)
         {
            color = glm::vec3(255.0f, 235.0f, 150.0f);
         }

         const bool checker = (((x >> 5) ^ (y >> 5)) & 1) != 0;
         const float panelLine = ((x % 64) == 0 || (y % 64) == 0) ? 0.78f : 1.0f;
         const float shade = (checker ? 0.95f : 1.0f) * panelLine;
         const size_t pixelOffset = static_cast<size_t>((y * width + x) * 4);
         pixels[pixelOffset + 0u] = ToByte(color.r * shade);
         pixels[pixelOffset + 1u] = ToByte(color.g * shade);
         pixels[pixelOffset + 2u] = ToByte(color.b * shade);
         pixels[pixelOffset + 3u] = 255u;
      }
   }

   const std::string texturePath = p_texturePath.generic_string();
   const int written = stbi_write_png(texturePath.c_str(), width, height, 4, pixels.data(), width * 4);
   ASSERT(written != 0, "Failed to write generated Cornell box texture");
}

TextureData LoadTexture(const std::filesystem::path& p_texturePath)
{
   int width = 0;
   int height = 0;
   int channels = 0;
   const std::string texturePath = p_texturePath.generic_string();
   stbi_uc* raw = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
   ASSERT(raw != nullptr, "Failed to load texture with stb_image");

   TextureData data;
   data.width = width;
   data.height = height;
   data.pixels.assign(raw, raw + static_cast<size_t>(width * height * 4));
   stbi_image_free(raw);
   return data;
}

std::filesystem::path ResolveAssetPath(const std::filesystem::path& p_baseDirectory, const std::string& p_path)
{
   std::filesystem::path path(p_path);
   if (path.is_relative())
   {
      path = p_baseDirectory / path;
   }
   return path.lexically_normal();
}

std::filesystem::path LoadDiffuseTexturePath(const std::filesystem::path& p_materialPath)
{
   std::ifstream materialFile(p_materialPath);
   ASSERT(materialFile.is_open(), "Failed to open material library");

   std::string line;
   while (std::getline(materialFile, line))
   {
      std::istringstream stream(line);
      std::string token;
      stream >> token;
      if (token == "map_Kd")
      {
         std::string texturePath;
         stream >> texturePath;
         ASSERT(!texturePath.empty(), "Material map_Kd entry had no texture path");
         return ResolveAssetPath(p_materialPath.parent_path(), texturePath);
      }
   }

   return {};
}

int ParseIndexPart(const std::string& p_text)
{
   if (p_text.empty())
   {
      return 0;
   }
   return std::stoi(p_text);
}

ObjVertexRef ParseObjVertexRef(const std::string& p_token)
{
   ObjVertexRef ref;
   const size_t firstSlash = p_token.find('/');
   if (firstSlash == std::string::npos)
   {
      ref.positionIndex = ParseIndexPart(p_token);
      return ref;
   }

   const size_t secondSlash = p_token.find('/', firstSlash + 1u);
   ref.positionIndex = ParseIndexPart(p_token.substr(0u, firstSlash));
   if (secondSlash == std::string::npos)
   {
      ref.texCoordIndex = ParseIndexPart(p_token.substr(firstSlash + 1u));
      return ref;
   }

   ref.texCoordIndex = ParseIndexPart(p_token.substr(firstSlash + 1u, secondSlash - firstSlash - 1u));
   ref.normalIndex = ParseIndexPart(p_token.substr(secondSlash + 1u));
   return ref;
}

size_t ResolveObjIndex(int p_index, size_t p_count)
{
   ASSERT(p_index != 0, "OBJ indices are 1-based and cannot be zero");
   const int64_t count = static_cast<int64_t>(p_count);
   const int64_t resolved = p_index > 0 ? static_cast<int64_t>(p_index - 1) : count + static_cast<int64_t>(p_index);
   ASSERT(resolved >= 0 && resolved < count, "OBJ index out of range");
   return static_cast<size_t>(resolved);
}

void AppendModelVertex(ModelData& p_model, const ObjVertexRef& p_ref, const std::vector<glm::vec3>& p_positions,
                       const std::vector<glm::vec2>& p_texCoords, const std::vector<glm::vec3>& p_normals)
{
   ASSERT(p_model.vertices.size() < static_cast<size_t>(std::numeric_limits<uint32_t>::max()), "Model is too large");

   const glm::vec3 position = p_positions[ResolveObjIndex(p_ref.positionIndex, p_positions.size())];
   const glm::vec2 texCoord =
       p_ref.texCoordIndex != 0 ? p_texCoords[ResolveObjIndex(p_ref.texCoordIndex, p_texCoords.size())] : glm::vec2(0.0f);
   const glm::vec3 normal =
       p_ref.normalIndex != 0 ? p_normals[ResolveObjIndex(p_ref.normalIndex, p_normals.size())] : glm::vec3(0.0f, 1.0f, 0.0f);

   Vertex vertex;
   vertex.position[0] = position.x;
   vertex.position[1] = position.y;
   vertex.position[2] = position.z;
   vertex.texCoord[0] = texCoord.x;
   vertex.texCoord[1] = texCoord.y;
   vertex.normal[0] = normal.x;
   vertex.normal[1] = normal.y;
   vertex.normal[2] = normal.z;

   p_model.indices.push_back(static_cast<uint32_t>(p_model.vertices.size()));
   p_model.vertices.push_back(vertex);
}

ModelData LoadObjModel(const std::filesystem::path& p_modelPath)
{
   std::ifstream modelFile(p_modelPath);
   ASSERT(modelFile.is_open(), "Failed to open OBJ model");

   ModelData model;
   std::vector<glm::vec3> positions;
   std::vector<glm::vec2> texCoords;
   std::vector<glm::vec3> normals;
   std::filesystem::path materialLibrary;

   std::string line;
   while (std::getline(modelFile, line))
   {
      std::istringstream stream(line);
      std::string token;
      stream >> token;

      if (token.empty() || token[0] == '#')
      {
         continue;
      }

      if (token == "mtllib")
      {
         std::string materialPath;
         stream >> materialPath;
         materialLibrary = ResolveAssetPath(p_modelPath.parent_path(), materialPath);
      }
      else if (token == "v")
      {
         glm::vec3 position;
         stream >> position.x >> position.y >> position.z;
         positions.push_back(position);
      }
      else if (token == "vt")
      {
         glm::vec2 texCoord;
         stream >> texCoord.x >> texCoord.y;
         texCoords.push_back(texCoord);
      }
      else if (token == "vn")
      {
         glm::vec3 normal;
         stream >> normal.x >> normal.y >> normal.z;
         normals.push_back(normal);
      }
      else if (token == "f")
      {
         std::vector<ObjVertexRef> faceRefs;
         std::string vertexToken;
         while (stream >> vertexToken)
         {
            faceRefs.push_back(ParseObjVertexRef(vertexToken));
         }

         ASSERT(faceRefs.size() >= 3u, "OBJ face must contain at least three vertices");
         for (size_t i = 1u; i + 1u < faceRefs.size(); i++)
         {
            AppendModelVertex(model, faceRefs[0u], positions, texCoords, normals);
            AppendModelVertex(model, faceRefs[i], positions, texCoords, normals);
            AppendModelVertex(model, faceRefs[i + 1u], positions, texCoords, normals);
         }
      }
   }

   ASSERT(!model.vertices.empty(), "OBJ model did not contain renderable geometry");

   if (!materialLibrary.empty())
   {
      model.texturePath = LoadDiffuseTexturePath(materialLibrary);
   }

   if (model.texturePath.empty())
   {
      model.texturePath = p_modelPath.parent_path().parent_path() / "Textures" / "cornell_box_atlas.png";
   }

   return model;
}

std::vector<uint8_t> LoadShaderBinary(const char* p_path)
{
   using namespace Foundation::IO;

   auto io = FileIO::CreateFileIO(
       FileIODescriptor{.m_path = p_path,
                        .m_fileIOFlags = Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});
   io->Open();
   const uint64_t size = io->GetFileSize();
   std::vector<uint8_t> binary(static_cast<size_t>(size));
   io->Read(binary.data(), size);
   return binary;
}

std::array<Ptr<Buffer>, 2u> CreateModelBuffers(GHI::ResourceFactory& p_factory, Ptr<GHI::Device> p_device,
                                               const ModelData& p_model)
{
   const uint64_t vertexBufferSize = static_cast<uint64_t>(p_model.vertices.size() * sizeof(Vertex));
   const uint64_t indexBufferSize = static_cast<uint64_t>(p_model.indices.size() * sizeof(uint32_t));

   Ptr<Buffer> vertexBuffer;
   {
      BufferDescriptor desc;
      desc.m_requestBufferSize = vertexBufferSize;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_bufferUsageFlags = BufferUsageFlags::TransferDestination | BufferUsageFlags::VertexBuffer;
      desc.m_queueFamilyAccess = QueueTypeFlags::GraphicsQueue;
      desc.m_initialData = p_model.vertices.data();
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
      desc.m_initialData = p_model.indices.data();
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

void RenderFunction(GHI::ResourceFactory& p_factory)
{
   std::vector<Ptr<GHI::PhysicalDevice>> physicalDevices = p_factory.GetPhysicalDevices();
   Ptr<GHI::PhysicalDevice> physicalDevice = SelectPhysicalDevice(physicalDevices);

   Ptr<GHI::Device> device = p_factory.CreateDevice(DeviceDescriptor{.m_physicalDevice = physicalDevice});

   Ptr<GHI::RenderWindow> renderWindow = p_factory.CreateRenderWindow(
       device, RenderWindowDescriptor{.m_windowResolution = glm::uvec2(1920u, 1080u), .m_windowTitle = "Model"});

   Ptr<GHI::Swapchain> swapchain = p_factory.CreateSwapchain(device, SwapchainDescriptor{.m_renderWindow = renderWindow});
   swapchain->Init();

   Ptr<GHI::ShaderModule> vertexShaderModule;
   Ptr<GHI::ShaderModule> fragmentShaderModule;
   {
      std::vector<uint8_t> vertBin = LoadShaderBinary("Data/Shaders/model.vert.spv");
      std::vector<uint8_t> fragBin = LoadShaderBinary("Data/Shaders/model.frag.spv");

      vertexShaderModule = p_factory.CreateShaderModule(
          device,
          ShaderModuleDescriptor{.m_spirvBinary = vertBin.data(), .m_binarySizeInBytes = static_cast<uint32_t>(vertBin.size())});
      fragmentShaderModule = p_factory.CreateShaderModule(
          device,
          ShaderModuleDescriptor{.m_spirvBinary = fragBin.data(), .m_binarySizeInBytes = static_cast<uint32_t>(fragBin.size())});
   }

   ModelData model = LoadObjModel("Data/Models/cornell_box.obj");
   EnsureCornellBoxTexture(model.texturePath);

   ASSERT(model.indices.size() <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()), "Model index count is too large");
   const uint32_t indexCount = static_cast<uint32_t>(model.indices.size());

   auto buffers = CreateModelBuffers(p_factory, device, model);
   Ptr<Buffer> vertexBuffer = buffers[0];
   Ptr<Buffer> indexBuffer = buffers[1];

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
      ASSERT(mappedMvp != nullptr, "Failed to map model uniform buffer");
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

   TextureData texData = LoadTexture(model.texturePath);

   Ptr<Image> textureImage;
   {
      ImageDescriptor desc;
      desc.m_imageUsageFlags = ImageUsageFlags::Sampled;
      desc.m_imageType = ImageType::Image2D;
      desc.m_extend = glm::uvec3(static_cast<uint32_t>(texData.width), static_cast<uint32_t>(texData.height), 1u);
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

   Ptr<GHI::Sampler> textureSampler =
       p_factory.CreateSampler(device, SamplerDescriptor{.m_magFilter = SamplerFilter::Linear,
                                                         .m_minFilter = SamplerFilter::Linear,
                                                         .m_mipmapMode = SamplerMipmapMode::Linear,
                                                         .m_addressModeU = SamplerAddressMode::Repeat,
                                                         .m_addressModeV = SamplerAddressMode::Repeat,
                                                         .m_addressModeW = SamplerAddressMode::Repeat});

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

   Ptr<GHI::VertexInputState> vertexInputState = p_factory.CreateVertexInputState(device, VertexInputStateDescriptor{});
   {
      GHI::VertexInputBinding& binding = vertexInputState->AddVertexInputBinding(VertexInputRate::VertexInputRateVertex);
      binding.m_stride = sizeof(Vertex);
      binding.AddVertexInputAttribute(0u, ResourceFormat::R32G32B32Sfloat, offsetof(Vertex, position));
      binding.AddVertexInputAttribute(1u, ResourceFormat::R32G32Sfloat, offsetof(Vertex, texCoord));
      binding.AddVertexInputAttribute(2u, ResourceFormat::R32G32B32Sfloat, offsetof(Vertex, normal));
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

   while (!renderWindow->ShouldClose())
   {
      const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();

      const uint64_t waitValue = static_cast<uint64_t>(std::max(static_cast<int64_t>(frameIndex) - maxFramesInFlight + 1, 0ll));
      PollEventsUntil([&submitFence, waitValue]() { return submitFence->IsValueSignaled(waitValue); });
      if (renderWindow->ShouldClose())
      {
         break;
      }

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
      {
         break;
      }

      {
         const glm::uvec2 res = renderWindow->GetWindowResolution();
         const float aspect = static_cast<float>(res.x) / static_cast<float>(res.y);

         Mvp mvp;
         mvp.projectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
         mvp.projectionMatrix[1][1] *= -1.0f;
         mvp.modelMatrix = glm::mat4(1.0f);
         mvp.viewMatrix =
             glm::lookAt(glm::vec3(0.0f, 1.05f, 3.05f), glm::vec3(0.0f, 0.95f, -0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
         memcpy(mappedMvp, &mvp, sizeof(Mvp));
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
         const RenderGraphResourceHandle textureResource =
             graph.ImportImageView("cornell atlas", textureImageView, ResourceUsage::SampledRead);

         auto [renderedSwapchainColor, renderedDepthStencil] =
             graph.AddPass("cornell box")
                 .Read("cornell atlas", textureResource, ResourceUsage::SampledRead)
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
                commandBuffer->SetStencilOp(StencilFaceFlags::FrontAndBack, StencilOp::Keep, StencilOp::Keep,
                                            StencilOp::Keep, CompareOp::Always);
                commandBuffer->SetRasterizerDiscardEnable(false);
                commandBuffer->SetDepthBiasEnable(false);
                commandBuffer->SetPrimitiveRestartEnable(false);

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
                   colorAttachment.m_clearValue = ClearColorValue{.m_clearValFloat = {0.025f, 0.03f, 0.035f, 1.0f}};

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

                commandBuffer->DrawIndexed(indexCount, 1u, 0u, 0u, 1u);
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
