#include <GHI/Vulkan/RenderGraph.h>

#include <algorithm>
#include <array>
#include <utility>

#include <Util/Assert.h>

#include <GHI/CommandBuffer.h>
#include <GHI/RenderCommands.h>
#include <GHI/ResourceFactory.h>
#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/BufferView.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/Image.h>
#include <GHI/Vulkan/ImageView.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

constexpr uint32_t IgnoredQueueFamily = static_cast<uint32_t>(-1);

struct VulkanTransientMemoryRequirements
{
   VkMemoryRequirements m_memoryRequirements = {};
   MemoryPropertyFlags m_memoryProperties = {};
};

class SharedDeviceMemory final
{
 public:
   SharedDeviceMemory(Ptr<Device> p_device, VkDeviceMemory p_memory) : m_device(std::move(p_device)), m_memory(p_memory)
   {
      ASSERT(m_device != nullptr, "Shared RenderGraph memory needs a Vulkan device");
      ASSERT(m_memory != VK_NULL_HANDLE, "Shared RenderGraph memory needs a native allocation");
   }

   ~SharedDeviceMemory()
   {
      vkFreeMemory(m_device->GetLogicalDeviceNative(), m_memory, nullptr);
   }

 private:
   Ptr<Device> m_device;
   VkDeviceMemory m_memory = VK_NULL_HANDLE;
};

VkImageCreateFlags ImageCreationFlagsToNative(ImageCreationFlags p_flags)
{
   VkImageCreateFlags nativeFlags = 0u;
   const uint32_t flags = static_cast<uint32_t>(p_flags);

   if ((flags & static_cast<uint32_t>(ImageCreationFlags::Alias)) != 0u)
   {
      nativeFlags |= VK_IMAGE_CREATE_ALIAS_BIT;
   }
   if ((flags & static_cast<uint32_t>(ImageCreationFlags::Cube_Or_CubeArray)) != 0u)
   {
      nativeFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
   }
   if ((flags & static_cast<uint32_t>(ImageCreationFlags::Array2D)) != 0u)
   {
      nativeFlags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
   }

   return nativeFlags;
}

VkImageUsageFlags ImageUsageFlagsToNative(ImageUsageFlags p_flags)
{
   VkImageUsageFlags nativeFlags = 0u;
   if (any(p_flags, ImageUsageFlags::TransferSource))
   {
      nativeFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
   }
   if (any(p_flags, ImageUsageFlags::TransferDestination))
   {
      nativeFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   }
   if (any(p_flags, ImageUsageFlags::Sampled))
   {
      nativeFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
   }
   if (any(p_flags, ImageUsageFlags::Storage))
   {
      nativeFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
   }
   if (any(p_flags, ImageUsageFlags::ColorAttachment))
   {
      nativeFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   if (any(p_flags, ImageUsageFlags::DepthStencilAttachment))
   {
      nativeFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   }
   if (any(p_flags, ImageUsageFlags::TransientAttachment))
   {
      nativeFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
   }
   if (any(p_flags, ImageUsageFlags::InputAttachment))
   {
      nativeFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
   }

   return nativeFlags;
}

bool NeedsShaderDeviceAddress(BufferUsageFlags p_usageFlags)
{
   return any(p_usageFlags, BufferUsageFlags::Uniform | BufferUsageFlags::Storage |
                                BufferUsageFlags::UniformTexel | BufferUsageFlags::StorageTexel);
}

VkImageCreateInfo CreateImageCreateInfo(const ImageDescriptor& p_desc, bool p_forceAlias)
{
   VkImageCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   createInfo.flags = ImageCreationFlagsToNative(p_desc.m_imageCreationFlags);
   if (p_forceAlias)
   {
      // Transient graph images are alias-capable because the materializer may bind several solved
      // lifetimes to the same allocation.
      createInfo.flags |= VK_IMAGE_CREATE_ALIAS_BIT;
   }
   createInfo.imageType = static_cast<VkImageType>(p_desc.m_imageType);
   createInfo.format = RenderTypeToNative::ResourceFormatToNative(p_desc.m_format);
   createInfo.extent = VkExtent3D{p_desc.m_extend.x, p_desc.m_extend.y, p_desc.m_extend.z};
   createInfo.mipLevels = p_desc.m_mipLevels;
   createInfo.arrayLayers = p_desc.m_arrayLayers;
   createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
   createInfo.tiling = static_cast<VkImageTiling>(p_desc.m_imageTiling);
   createInfo.usage = ImageUsageFlagsToNative(p_desc.m_imageUsageFlags);
   createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   createInfo.initialLayout = static_cast<VkImageLayout>(p_desc.m_initialLayout);
   return createInfo;
}

VkBufferCreateInfo CreateBufferCreateInfo(const BufferDescriptor& p_desc)
{
   VkBufferCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   createInfo.size = std::max<uint64_t>(p_desc.m_requestBufferSize, 1u);
   createInfo.usage = RenderTypeToNative::BufferUsageFlagsToNative(p_desc.m_bufferUsageFlags);
   if (NeedsShaderDeviceAddress(p_desc.m_bufferUsageFlags))
   {
      createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
   }
   createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   return createInfo;
}

VulkanTransientMemoryRequirements QueryImageRequirements(const ImageDescriptor& p_desc, Ptr<Device> p_device)
{
   const VkImageCreateInfo createInfo = CreateImageCreateInfo(p_desc, true);
   const VkMemoryRequirements requirements = p_device->GetImageMemoryRequirements(createInfo);
   return VulkanTransientMemoryRequirements{.m_memoryRequirements = requirements,
                                            .m_memoryProperties = p_desc.m_memoryProperties};
}

VulkanTransientMemoryRequirements QueryBufferRequirements(const BufferDescriptor& p_desc, Ptr<Device> p_device)
{
   const VkBufferCreateInfo createInfo = CreateBufferCreateInfo(p_desc);
   const VkMemoryRequirements requirements = p_device->GetBufferMemoryRequirements(createInfo);
   return VulkanTransientMemoryRequirements{.m_memoryRequirements = requirements,
                                            .m_memoryProperties = p_desc.m_memoryProperties};
}

VulkanTransientMemoryRequirements QueryTransientRequirements(const GHI::RenderGraph& p_renderGraph,
                                                             Ptr<Device> p_device,
                                                             RenderGraphResourceHandle p_handle)
{
   if (p_renderGraph.GetResourceType(p_handle) == RenderGraphResourceType::Image)
   {
      const ImageDescriptor* desc = p_renderGraph.GetImageDescriptor(p_handle);
      ASSERT(desc != nullptr, "RenderGraph image transient requirement query needs an ImageDescriptor");
      return QueryImageRequirements(*desc, std::move(p_device));
   }

   ASSERT(p_renderGraph.GetResourceType(p_handle) == RenderGraphResourceType::Buffer,
          "Unsupported RenderGraph resource type for Vulkan transient requirements");
   const BufferDescriptor* desc = p_renderGraph.GetBufferDescriptor(p_handle);
   ASSERT(desc != nullptr, "RenderGraph buffer transient requirement query needs a BufferDescriptor");
   return QueryBufferRequirements(*desc, std::move(p_device));
}

VulkanTransientMemoryRequirements QueryTransientRequirements(const RenderGraphTransientResourceRequest& p_request,
                                                             Ptr<Device> p_device)
{
   if (p_request.m_type == RenderGraphResourceType::Image)
   {
      ASSERT(p_request.m_imageDesc != nullptr, "RenderGraph image transient request needs an ImageDescriptor");
      return QueryImageRequirements(*p_request.m_imageDesc, std::move(p_device));
   }

   ASSERT(p_request.m_type == RenderGraphResourceType::Buffer,
          "Unsupported RenderGraph resource type for Vulkan transient requirements");
   ASSERT(p_request.m_bufferDesc != nullptr, "RenderGraph buffer transient request needs a BufferDescriptor");
   return QueryBufferRequirements(*p_request.m_bufferDesc, std::move(p_device));
}

uint32_t GetCompatibleMemoryTypeBits(const VulkanTransientMemoryRequirements& p_requirements, Ptr<Device> p_device)
{
   return p_device->GetCompatibleMemoryTypeBits(p_requirements.m_memoryRequirements.memoryTypeBits,
                                                p_requirements.m_memoryProperties);
}

ImageAspectFlags GetDefaultAspectMask(ResourceFormat p_format)
{
   switch (p_format)
   {
   case ResourceFormat::D16Unorm:
   case ResourceFormat::X8D24UnormPack32:
   case ResourceFormat::D32Sfloat:
      return ImageAspectFlags::Depth;
   case ResourceFormat::S8Uint:
      return ImageAspectFlags::Stencil;
   case ResourceFormat::D16UnormS8Uint:
   case ResourceFormat::D24UnormS8Uint:
   case ResourceFormat::D32SfloatS8Uint:
      return ImageAspectFlags::Depth | ImageAspectFlags::Stencil;
   default:
      return ImageAspectFlags::Color;
   }
}

ImageViewType GetDefaultImageViewType(const ImageDescriptor& p_desc)
{
   switch (p_desc.m_imageType)
   {
   case ImageType::Image1D:
      return p_desc.m_arrayLayers > 1u ? ImageViewType::View1DArray : ImageViewType::View1D;
   case ImageType::Image2D:
      return p_desc.m_arrayLayers > 1u ? ImageViewType::View2DArray : ImageViewType::View2D;
   case ImageType::Image3D:
      return ImageViewType::View3D;
   default:
      ASSERT(false, "Unsupported RenderGraph image type for default Vulkan ImageView");
      return ImageViewType::View2D;
   }
}

ImageViewDescriptor CreateDefaultImageViewDescriptor(Ptr<GHI::Image> p_image, const ImageDescriptor& p_desc)
{
   return ImageViewDescriptor{.m_image = std::move(p_image),
                              .m_extend = p_desc.m_extend,
                              .m_viewType = GetDefaultImageViewType(p_desc),
                              .m_format = p_desc.m_format,
                              .m_baseMipLevel = 0u,
                              .m_mipLevelCount = p_desc.m_mipLevels,
                              .m_baseArrayLayer = 0u,
                              .m_arrayLayerCount = p_desc.m_arrayLayers,
                              .m_aspectMask = GetDefaultAspectMask(p_desc.m_format)};
}

BufferUsage GetDefaultBufferViewUsage(BufferUsageFlags p_usageFlags)
{
   if (any(p_usageFlags, BufferUsageFlags::Storage))
   {
      return BufferUsage::Storage;
   }
   if (any(p_usageFlags, BufferUsageFlags::Uniform))
   {
      return BufferUsage::Uniform;
   }
   if (any(p_usageFlags, BufferUsageFlags::StorageTexel))
   {
      return BufferUsage::StorageTexel;
   }
   if (any(p_usageFlags, BufferUsageFlags::UniformTexel))
   {
      return BufferUsage::UniformTexel;
   }
   if (any(p_usageFlags, BufferUsageFlags::VertexBuffer))
   {
      return BufferUsage::VertexBuffer;
   }
   if (any(p_usageFlags, BufferUsageFlags::IndexBuffer))
   {
      return BufferUsage::IndexBuffer;
   }
   if (any(p_usageFlags, BufferUsageFlags::IndirectBuffer))
   {
      return BufferUsage::IndirectBuffer;
   }
   if (any(p_usageFlags, BufferUsageFlags::TransferDestination))
   {
      return BufferUsage::TransferDestination;
   }
   if (any(p_usageFlags, BufferUsageFlags::TransferSource))
   {
      return BufferUsage::TransferSource;
   }

   ASSERT(false, "Unsupported RenderGraph buffer usage for default Vulkan BufferView");
   return BufferUsage::Invalid;
}

BufferViewDescriptor CreateDefaultBufferViewDescriptor(Ptr<GHI::Buffer> p_buffer, const BufferDescriptor& p_desc)
{
   return BufferViewDescriptor{.m_buffer = std::move(p_buffer),
                               .m_format = ResourceFormat::Invalid,
                               .m_offsetFromBaseAddress = 0u,
                               .m_bufferViewRange = p_desc.m_requestBufferSize,
                               .m_usage = GetDefaultBufferViewUsage(p_desc.m_bufferUsageFlags)};
}

Ptr<GHI::ImageView> CreateStandaloneImageView(Ptr<Device> p_device, const ImageDescriptor& p_desc)
{
   ImageDescriptor imageDesc = p_desc;
   Ptr<GHI::Image> image = ResourceFactory::Get()->CreateImage(p_device, std::move(imageDesc));
   return ResourceFactory::Get()->CreateImageView(p_device, CreateDefaultImageViewDescriptor(image, p_desc));
}

Ptr<GHI::BufferView> CreateStandaloneBufferView(Ptr<Device> p_device, const BufferDescriptor& p_desc)
{
   BufferDescriptor bufferDesc = p_desc;
   Ptr<GHI::Buffer> buffer = ResourceFactory::Get()->CreateBuffer(p_device, std::move(bufferDesc));
   return ResourceFactory::Get()->CreateBufferView(p_device, CreateDefaultBufferViewDescriptor(buffer, p_desc));
}

uint32_t GetSharedCompatibleMemoryTypeBits(Ptr<Device> p_device, const GHI::RenderGraph& p_renderGraph,
                                           const std::vector<RenderGraphResourceHandle>& p_resources)
{
   ASSERT(!p_resources.empty(), "Can't check shared RenderGraph memory compatibility for an empty resource list");

   uint32_t memoryTypeBits = 0u;
   bool hasMemoryTypeBits = false;
   for (const RenderGraphResourceHandle resource : p_resources)
   {
      const VulkanTransientMemoryRequirements requirements =
          QueryTransientRequirements(p_renderGraph, p_device, resource);
      const uint32_t compatibleTypeBits = GetCompatibleMemoryTypeBits(requirements, p_device);
      memoryTypeBits = hasMemoryTypeBits ? memoryTypeBits & compatibleTypeBits : compatibleTypeBits;
      hasMemoryTypeBits = true;

      if (memoryTypeBits == 0u)
      {
         return 0u;
      }
   }

   return memoryTypeBits;
}

uint32_t GetSharedCompatibleMemoryTypeBits(Ptr<Device> p_device,
                                           std::span<const RenderGraphTransientResourceRequest> p_resources)
{
   ASSERT(!p_resources.empty(), "Can't check shared RenderGraph memory compatibility for an empty resource list");

   uint32_t memoryTypeBits = 0u;
   bool hasMemoryTypeBits = false;
   for (const RenderGraphTransientResourceRequest& resource : p_resources)
   {
      const VulkanTransientMemoryRequirements requirements = QueryTransientRequirements(resource, p_device);
      const uint32_t compatibleTypeBits = GetCompatibleMemoryTypeBits(requirements, p_device);
      memoryTypeBits = hasMemoryTypeBits ? memoryTypeBits & compatibleTypeBits : compatibleTypeBits;
      hasMemoryTypeBits = true;

      if (memoryTypeBits == 0u)
      {
         return 0u;
      }
   }

   return memoryTypeBits;
}

VkDeviceMemory AllocateSharedMemory(Ptr<Device> p_device,
                                    std::span<const RenderGraphTransientResourceRequest> p_resources,
                                    uint64_t& p_allocatedSize)
{
   ASSERT(!p_resources.empty(), "Can't allocate shared RenderGraph memory for an empty resource list");

   uint64_t allocationSize = 0u;
   uint64_t allocationAlignment = 1u;
   MemoryPropertyFlags memoryProperties = {};
   bool hasMemoryProperties = false;
   VkMemoryAllocateFlags allocateFlags = 0u;
   uint32_t memoryTypeBits = GetSharedCompatibleMemoryTypeBits(p_device, p_resources);
   if (memoryTypeBits == 0u)
   {
      return VK_NULL_HANDLE;
   }

   for (const RenderGraphTransientResourceRequest& resource : p_resources)
   {
      const VulkanTransientMemoryRequirements requirements = QueryTransientRequirements(resource, p_device);

      allocationSize = std::max(allocationSize, requirements.m_memoryRequirements.size);
      allocationAlignment = std::max(allocationAlignment, requirements.m_memoryRequirements.alignment);
      if (!hasMemoryProperties)
      {
         memoryProperties = requirements.m_memoryProperties;
         hasMemoryProperties = true;
      }

      if (resource.m_type == RenderGraphResourceType::Buffer)
      {
         const BufferDescriptor* desc = resource.m_bufferDesc;
         ASSERT(desc != nullptr, "RenderGraph transient buffer needs a BufferDescriptor");
         if (NeedsShaderDeviceAddress(desc->m_bufferUsageFlags))
         {
            allocateFlags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
         }
      }
   }

   VkMemoryRequirements allocationRequirements = {};
   allocationRequirements.size = allocationSize;
   allocationRequirements.alignment = allocationAlignment;
   allocationRequirements.memoryTypeBits = memoryTypeBits;

   auto [memory, allocatedSize] = p_device->AllocateDeviceMemory(allocationRequirements, memoryProperties, allocateFlags);
   p_allocatedSize = allocatedSize;
   return memory;
}

void MaterializeImageIntoMemory(RenderGraphTransientResourceWriter& p_writer, Ptr<Device> p_device,
                                const RenderGraphTransientResourceRequest& p_request, VkDeviceMemory p_memory,
                                uint64_t p_allocatedSize, std::shared_ptr<void> p_memoryOwner)
{
   const ImageDescriptor* desc = p_request.m_imageDesc;
   ASSERT(desc != nullptr, "RenderGraph transient image needs an ImageDescriptor");

   VkImageCreateInfo createInfo = CreateImageCreateInfo(*desc, true);
   VkImage imageNative = VK_NULL_HANDLE;
   const VkResult createResult = vkCreateImage(p_device->GetLogicalDeviceNative(), &createInfo, nullptr, &imageNative);
   ASSERT(createResult == VK_SUCCESS, "Failed to create a RenderGraph transient image");

   const VkResult bindResult = vkBindImageMemory(p_device->GetLogicalDeviceNative(), imageNative, p_memory, 0u);
   ASSERT(bindResult == VK_SUCCESS, "Failed to bind RenderGraph transient image memory");

   ImageDescriptor imageDesc = *desc;
   Ptr<GHI::Image> image =
       std::make_shared<Image>(p_device, std::move(imageDesc), imageNative, p_memory, p_allocatedSize, p_memoryOwner);
   p_writer.SetImage(p_request.m_handle, std::move(image));
}

void MaterializeBufferIntoMemory(RenderGraphTransientResourceWriter& p_writer, Ptr<Device> p_device,
                                 const RenderGraphTransientResourceRequest& p_request, VkDeviceMemory p_memory,
                                 uint64_t p_allocatedSize, std::shared_ptr<void> p_memoryOwner)
{
   const BufferDescriptor* desc = p_request.m_bufferDesc;
   ASSERT(desc != nullptr, "RenderGraph transient buffer needs a BufferDescriptor");

   VkBufferCreateInfo createInfo = CreateBufferCreateInfo(*desc);
   VkBuffer bufferNative = VK_NULL_HANDLE;
   const VkResult createResult = vkCreateBuffer(p_device->GetLogicalDeviceNative(), &createInfo, nullptr, &bufferNative);
   ASSERT(createResult == VK_SUCCESS, "Failed to create a RenderGraph transient buffer");

   const VkResult bindResult = vkBindBufferMemory(p_device->GetLogicalDeviceNative(), bufferNative, p_memory, 0u);
   ASSERT(bindResult == VK_SUCCESS, "Failed to bind RenderGraph transient buffer memory");

   BufferDescriptor bufferDesc = *desc;
   Ptr<GHI::Buffer> buffer =
       std::make_shared<Buffer>(p_device, std::move(bufferDesc), bufferNative, p_memory, p_allocatedSize, p_memoryOwner);
   p_writer.SetBuffer(p_request.m_handle, std::move(buffer));
}

void MaterializeTransientResourceGroup(RenderGraphTransientResourceWriter& p_writer, Ptr<Device> p_device,
                                       std::span<const RenderGraphTransientResourceRequest> p_resources)
{
   uint64_t allocatedSize = 0u;
   VkDeviceMemory memory = AllocateSharedMemory(p_device, p_resources, allocatedSize);
   if (memory == VK_NULL_HANDLE)
   {
      // ConfigureRenderGraph installs a group compatibility checker that should prevent this path.
      // Keep the split as a defensive fallback for future Vulkan constraints that are not part of scheduling yet.
      ASSERT(p_resources.size() > 1u, "RenderGraph transient resource has no compatible Vulkan memory type");
      for (const RenderGraphTransientResourceRequest& resource : p_resources)
      {
         std::array<RenderGraphTransientResourceRequest, 1u> singleResource{resource};
         MaterializeTransientResourceGroup(p_writer, p_device, singleResource);
      }
      return;
   }

   std::shared_ptr<void> memoryOwner = std::make_shared<SharedDeviceMemory>(p_device, memory);
   for (const RenderGraphTransientResourceRequest& resource : p_resources)
   {
      if (resource.m_type == RenderGraphResourceType::Image)
      {
         MaterializeImageIntoMemory(p_writer, p_device, resource, memory, allocatedSize, memoryOwner);
         continue;
      }

      ASSERT(resource.m_type == RenderGraphResourceType::Buffer, "Unsupported RenderGraph transient resource type");
      MaterializeBufferIntoMemory(p_writer, p_device, resource, memory, allocatedSize, memoryOwner);
   }
}

void MaterializeTransientAliases(std::span<const RenderGraphTransientAliasGroupRequest> p_groups,
                                 RenderGraphTransientResourceWriter& p_writer, Ptr<Device> p_device)
{
   for (const RenderGraphTransientAliasGroupRequest& group : p_groups)
   {
      if (!group.m_resources.empty())
      {
         MaterializeTransientResourceGroup(p_writer, p_device, group.m_resources);
      }
   }
}

void EmitBarrier(CommandBuffer& p_commandBuffer, const RenderGraphBarrierInfo& p_barrierInfo)
{
   ASSERT(!p_barrierInfo.RequiresQueueFamilyOwnershipTransfer(),
          "Cross-queue RenderGraph ownership transfers need multi-queue execution support");

   const ResourceUsageInfo oldInfo =
       ResourceUsageToInfo(p_barrierInfo.m_oldUsage, p_barrierInfo.m_oldShaderStages);
   const ResourceUsageInfo newInfo =
       ResourceUsageToInfo(p_barrierInfo.m_newUsage, p_barrierInfo.m_newShaderStages);

   PipelineBarrierCommand* barrier = p_commandBuffer.PipelineBarrier();

   if (p_barrierInfo.m_resourceType == RenderGraphResourceType::Image)
   {
      ASSERT(oldInfo.m_imageLayout != ImageLayout::Invalid, "Old ResourceUsage is not valid for image barriers");
      ASSERT(newInfo.m_imageLayout != ImageLayout::Invalid, "New ResourceUsage is not valid for image barriers");
      ASSERT(p_barrierInfo.m_imageView != nullptr, "RenderGraph image resource has no materialized ImageView");

      barrier->AddImageBarrier(oldInfo.m_pipelineStages, oldInfo.m_access, newInfo.m_pipelineStages, newInfo.m_access,
                               oldInfo.m_imageLayout, newInfo.m_imageLayout, IgnoredQueueFamily, IgnoredQueueFamily,
                               p_barrierInfo.m_imageView);
      return;
   }

   ASSERT(p_barrierInfo.m_resourceType == RenderGraphResourceType::Buffer, "Unsupported RenderGraph resource type");
   ASSERT(p_barrierInfo.m_bufferView != nullptr, "RenderGraph buffer resource has no materialized BufferView");
   barrier->AddBufferBarrier(oldInfo.m_pipelineStages, oldInfo.m_access, newInfo.m_pipelineStages, newInfo.m_access,
                             IgnoredQueueFamily, IgnoredQueueFamily, p_barrierInfo.m_bufferView);
}

} // namespace

void ConfigureRenderGraph(GHI::RenderGraph& p_renderGraph, Ptr<GHI::Device> p_device)
{
   ASSERT(p_device != nullptr, "Vulkan RenderGraph configuration needs a Device");

   Ptr<Device> device = Cast<Device>(p_device);
   GHI::RenderGraph* renderGraph = &p_renderGraph;

   p_renderGraph.SetBarrierEmitter(
       [](CommandBuffer& p_commandBuffer, const RenderGraphBarrierInfo& p_barrierInfo) {
          EmitBarrier(p_commandBuffer, p_barrierInfo);
       });
   p_renderGraph.SetQueueFamilyResolver([device](QueueFamilyType p_queueType) {
      return device->GetQueueFamilyInfo(p_queueType);
   });
   p_renderGraph.SetImageMaterializer([device](const ImageDescriptor& p_desc, [[maybe_unused]] bool p_canBeTransient) {
      return CreateStandaloneImageView(device, p_desc);
   });
   p_renderGraph.SetBufferMaterializer([device](const BufferDescriptor& p_desc, [[maybe_unused]] bool p_canBeTransient) {
      return CreateStandaloneBufferView(device, p_desc);
   });
   p_renderGraph.SetImageViewMaterializer([](Ptr<GHI::Image> p_image, const ImageDescriptor& p_desc) {
      Ptr<GHI::Device> device = p_image->GetDevice();
      return ResourceFactory::Get()->CreateImageView(
          device, CreateDefaultRenderGraphImageViewDescriptor(std::move(p_image), p_desc));
   });
   p_renderGraph.SetBufferViewMaterializer([](Ptr<GHI::Buffer> p_buffer, const BufferDescriptor& p_desc) {
      Ptr<GHI::Device> device = p_buffer->GetDevice();
      return ResourceFactory::Get()->CreateBufferView(
          device, CreateDefaultRenderGraphBufferViewDescriptor(std::move(p_buffer), p_desc));
   });
   p_renderGraph.SetTransientAllocationSizeResolver(
       [renderGraph, device](RenderGraphResourceHandle p_handle) -> uint64_t {
          return QueryTransientRequirements(*renderGraph, device, p_handle).m_memoryRequirements.size;
       });
   p_renderGraph.SetTransientCompatibilityChecker(
       [renderGraph, device](RenderGraphResourceHandle p_first, RenderGraphResourceHandle p_second) {
          const VulkanTransientMemoryRequirements firstRequirements =
              QueryTransientRequirements(*renderGraph, device, p_first);
          const VulkanTransientMemoryRequirements secondRequirements =
              QueryTransientRequirements(*renderGraph, device, p_second);

          const uint32_t firstMemoryTypeBits = GetCompatibleMemoryTypeBits(firstRequirements, device);
          const uint32_t secondMemoryTypeBits = GetCompatibleMemoryTypeBits(secondRequirements, device);
          return (firstMemoryTypeBits & secondMemoryTypeBits) != 0u;
       });
   p_renderGraph.SetTransientAliasGroupCompatibilityChecker(
       [renderGraph, device](const std::vector<RenderGraphResourceHandle>& p_groupResources,
                             RenderGraphResourceHandle p_candidate) {
          std::vector<RenderGraphResourceHandle> resources = p_groupResources;
          resources.push_back(p_candidate);
          return GetSharedCompatibleMemoryTypeBits(device, *renderGraph, resources) != 0u;
       });
   p_renderGraph.SetTransientMaterializer(
       [device](std::span<const RenderGraphTransientAliasGroupRequest> p_groups,
                RenderGraphTransientResourceWriter& p_writer) {
          MaterializeTransientAliases(p_groups, p_writer, device);
       });
   p_renderGraph.SetSubCommandBufferCreator([device]([[maybe_unused]] Ptr<GHI::Device> p_device) {
      return ResourceFactory::Get()->CreateSubCommandBuffer(device, SubCommandBufferDescriptor{});
   });
   p_renderGraph.SetQueryReadbackBufferCreator([device]([[maybe_unused]] Ptr<GHI::Device> p_device,
                                                        const BufferDescriptor& p_desc) {
      BufferDescriptor desc = p_desc;
      return ResourceFactory::Get()->CreateBuffer(device, std::move(desc));
   });
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
