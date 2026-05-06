#include <GHI/Vulkan/Image.h>

#include <unordered_map>

#include <Util/Util.h>

#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/RendererTypes.h>
#include <GHI/Vulkan/Swapchain.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

namespace Internal
{

VkDevice GetNativeDevice(Ptr<GHI::Device> p_device)
{
   return Cast<Vulkan::Device>(p_device)->GetLogicalDevice();
}

} // namespace Internal

} // namespace

Image::Image(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc) : GHI::Image(p_device, std::move(p_desc))
{
   m_extend = VkExtent3D{GetDesc().m_extend.x, GetDesc().m_extend.y, GetDesc().m_extend.z};
   m_format = RenderTypeToNative::ResourceFormatToNative(GetDesc().m_format);
   m_imageType = static_cast<VkImageType>(GetDesc().m_imageType);
   m_imageCreationFlags = GetDesc().m_imageCreationFlags;
   m_imageUsageFlags = GetDesc().m_imageUsageFlags;
   m_mipLevels = GetDesc().m_mipLevels;
   m_arrayLayers = GetDesc().m_arrayLayers;
   m_imageTiling = static_cast<VkImageTiling>(GetDesc().m_imageTiling);
   m_initialLayout = static_cast<VkImageLayout>(GetDesc().m_initialLayout);
   m_memoryProperties = GetDesc().m_memoryProperties;

   VkImageCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   createInfo.pNext = nullptr;
   createInfo.flags = ImageCreationFlagsToNative(m_imageCreationFlags);
   createInfo.imageType = m_imageType;
   createInfo.format = m_format;
   createInfo.extent = m_extend;
   createInfo.mipLevels = m_mipLevels;
   createInfo.arrayLayers = m_arrayLayers;
   // TODO: don't support multi sampling for now
   createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
   createInfo.tiling = m_imageTiling;
   createInfo.usage = ImageUsageFlagsToNative(m_imageUsageFlags);
   // For now, only allow a single QueueFamilyIndex access at a time
   createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   createInfo.queueFamilyIndexCount = 0u;
   createInfo.pQueueFamilyIndices = nullptr;
   createInfo.initialLayout = m_initialLayout;

   VkDevice nativeLogicalDevice = Internal::GetNativeDevice(m_device);

   VkResult res = vkCreateImage(nativeLogicalDevice, &createInfo, nullptr, &m_imageNative);
   ASSERT(res == VK_SUCCESS, "Failed to create the Image resource");

   VkMemoryRequirements memoryRequirements;
   vkGetImageMemoryRequirements(nativeLogicalDevice, m_imageNative, &memoryRequirements);
   auto [deviceMemory, allocatedMemory] =
       Cast<Vulkan::Device>(m_device)->AllocateDeviceMemory(memoryRequirements, m_memoryProperties);
   m_deviceMemory = deviceMemory;
   m_bufferSizeAllocatedMemory = allocatedMemory;

   // Bind the Buffer resource to the Memory resource
   res = vkBindImageMemory(nativeLogicalDevice, GetImageNative(), GetDeviceMemoryNative(), 0u);
   ASSERT(res == VK_SUCCESS, "Failed to bind the Buffer resource to the Memory resource");
}

Image::Image(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc, VkImage p_imageNative, VkDeviceMemory p_deviceMemory,
             uint64_t p_allocatedMemory, std::shared_ptr<void> p_memoryOwner)
    : GHI::Image(p_device, std::move(p_desc))
{
   ASSERT(p_imageNative != VK_NULL_HANDLE, "Aliased image must have a valid native handle");
   ASSERT(p_deviceMemory != VK_NULL_HANDLE, "Aliased image must have valid device memory");

   m_extend = VkExtent3D{GetDesc().m_extend.x, GetDesc().m_extend.y, GetDesc().m_extend.z};
   m_format = RenderTypeToNative::ResourceFormatToNative(GetDesc().m_format);
   m_imageType = static_cast<VkImageType>(GetDesc().m_imageType);
   m_imageCreationFlags = GetDesc().m_imageCreationFlags;
   m_imageUsageFlags = GetDesc().m_imageUsageFlags;
   m_mipLevels = GetDesc().m_mipLevels;
   m_arrayLayers = GetDesc().m_arrayLayers;
   m_imageTiling = static_cast<VkImageTiling>(GetDesc().m_imageTiling);
   m_initialLayout = static_cast<VkImageLayout>(GetDesc().m_initialLayout);
   m_memoryProperties = GetDesc().m_memoryProperties;
   m_imageNative = p_imageNative;
   m_deviceMemory = p_deviceMemory;
   m_bufferSizeAllocatedMemory = p_allocatedMemory;
   m_memoryOwner = std::move(p_memoryOwner);
}

Image::Image(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc, VkImage p_imageNative, Swapchain* p_swapchain,
             uint32_t p_swapchainIndex)
    : GHI::Image(p_device, std::move(p_desc))
{
   ASSERT(p_imageNative != VK_NULL_HANDLE, "Swapchain image must have a valid native handle");
   ASSERT(p_swapchain != nullptr, "Swapchain image must reference its owning swapchain");

   m_extend = VkExtent3D{GetDesc().m_extend.x, GetDesc().m_extend.y, GetDesc().m_extend.z};
   m_format = RenderTypeToNative::ResourceFormatToNative(GetDesc().m_format);
   m_imageType = static_cast<VkImageType>(GetDesc().m_imageType);
   m_imageCreationFlags = GetDesc().m_imageCreationFlags;
   m_imageUsageFlags = GetDesc().m_imageUsageFlags;
   m_mipLevels = GetDesc().m_mipLevels;
   m_arrayLayers = GetDesc().m_arrayLayers;
   m_imageTiling = static_cast<VkImageTiling>(GetDesc().m_imageTiling);
   m_initialLayout = static_cast<VkImageLayout>(GetDesc().m_initialLayout);
   m_memoryProperties = GetDesc().m_memoryProperties;
   m_swapchain = p_swapchain;
   m_swapchainIndex = p_swapchainIndex;
   m_imageNative = p_imageNative;
}

Image::~Image()
{
   // Only clean up the Vulkan resource if it's not created from a swapchain
   if (!m_swapchain && m_imageNative != VK_NULL_HANDLE)
   {
      VkDevice nativeDevice = Cast<Vulkan::Device>(m_device)->GetLogicalDeviceNative();
      vkDestroyImage(nativeDevice, m_imageNative, nullptr);
      if (GetDeviceMemoryNative() != VK_NULL_HANDLE && m_memoryOwner == nullptr)
      {
         vkFreeMemory(nativeDevice, GetDeviceMemoryNative(), nullptr);
      }
   }
}

bool Image::IsSwapchainImage() const
{
   return m_swapchain != nullptr;
}

VkImage Image::GetImageNative() const
{
   return m_imageNative;
}

VkFormat Image::GetImageFormatNative() const
{
   return m_format;
}

VkExtent3D Image::GetImageExtendNative() const
{
   return m_extend;
}

VkImageTiling Image::GetImageTilingNative() const
{
   return m_imageTiling;
}

const VkDeviceMemory Image::GetDeviceMemoryNative() const
{
   return m_deviceMemory;
}

VkImageCreateFlagBits Image::ImageCreationFlagsToNative(ImageCreationFlags p_flags)
{
   static const std::unordered_map<ImageCreationFlags, VkImageCreateFlagBits> ImageCreationFlagsToNativeMap = {
       {ImageCreationFlags::Alias, VK_IMAGE_CREATE_ALIAS_BIT},
       {ImageCreationFlags::Cube_Or_CubeArray, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT},
       {ImageCreationFlags::Array2D, VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkImageCreateFlagBits>(ImageCreationFlagsToNativeMap, p_flags);
}

VkImageUsageFlagBits Image::ImageUsageFlagsToNative(ImageUsageFlags p_flags)
{
   static const std::unordered_map<ImageUsageFlags, VkImageUsageFlagBits> ImageUsageFlagsToNativeMap = {
       {ImageUsageFlags::TransferSource, VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
       {ImageUsageFlags::TransferDestination, VK_IMAGE_USAGE_TRANSFER_DST_BIT},
       {ImageUsageFlags::Sampled, VK_IMAGE_USAGE_SAMPLED_BIT},
       {ImageUsageFlags::Storage, VK_IMAGE_USAGE_STORAGE_BIT},
       {ImageUsageFlags::ColorAttachment, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
       {ImageUsageFlags::DepthStencilAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
       {ImageUsageFlags::TransientAttachment, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT},
       {ImageUsageFlags::InputAttachment, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkImageUsageFlagBits>(ImageUsageFlagsToNativeMap, p_flags);
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
