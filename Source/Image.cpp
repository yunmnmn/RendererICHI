#include <Image.h>

#include <std/unordered_map_bootstrap.h>

#include <Util/Util.h>

#include <Renderer.h>
#include <VulkanDevice.h>
#include <Swapchain.h>

namespace Render
{
Image::Image(ImageDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_extend = p_desc.m_extend;
   m_format = p_desc.m_format;
   m_imageType = p_desc.m_imageType;
   m_imageCreationFlags = p_desc.m_imageCreationFlags;
   m_imageUsageFlags = p_desc.m_imageUsageFlags;
   m_mipLevels = p_desc.m_mipLevels;
   m_arrayLayers = p_desc.m_arrayLayers;
   m_imageTiling = p_desc.m_imageTiling;
   m_initialLayout = p_desc.m_initialLayout;
   m_memoryProperties = p_desc.m_memoryProperties;

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
   createInfo.tiling = p_desc.m_imageTiling;
   createInfo.usage = ImageUsageFlagsToNative(m_imageUsageFlags);
   // For now, only allow a single QueueFamilyIndex access at a time
   createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   createInfo.queueFamilyIndexCount = 0u;
   createInfo.pQueueFamilyIndices = nullptr;
   createInfo.initialLayout = m_initialLayout;

   VkResult res = vkCreateImage(m_vulkanDeviceRef->GetLogicalDeviceNative(), &createInfo, nullptr, &m_imageNative);
   ASSERT(res == VK_SUCCESS, "Failed to create the Image resource");

   VkMemoryRequirements memoryRequirements;
   vkGetImageMemoryRequirements(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_imageNative, &memoryRequirements);
   auto [deviceMemory, allocatedMemory] = m_vulkanDeviceRef->AllocateDeviceMemory(memoryRequirements, m_memoryProperties);
   m_deviceMemory = deviceMemory;
   m_bufferSizeAllocatedMemory = allocatedMemory;

   // Bind the Buffer resource to the Memory resource
   res = vkBindImageMemory(m_vulkanDeviceRef->GetLogicalDeviceNative(), GetImageNative(), GetDeviceMemoryNative(), 0u);
   ASSERT(res == VK_SUCCESS, "Failed to bind the Buffer resource to the Memory resource");
}

Image::Image(ImageDescriptor2&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_swapchainRef = p_desc.m_swapchainRef;
   m_swapchainIndex = p_desc.m_swapchainIndex;

   VkExtent2D extend = m_swapchainRef->GetExtend();

   m_imageNative = m_swapchainRef->GetSwapchainImageNative(m_swapchainIndex);
   // TODO: this might be wrong
   m_extend = VkExtent3D{.width = extend.width, .height = extend.height, .depth = 1u};
   m_format = m_swapchainRef->GetFormat();
}

Image::~Image()
{
   // Only clean up the Vulkan resource if it's not created from a swapchain
   if (!m_swapchainRef.IsInitialized())
   {
      vkDestroyImage(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_imageNative, nullptr);
   }
}

bool Image::IsSwapchainImage() const
{
   return m_swapchainRef.IsInitialized();
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

const VkDeviceMemory Image::GetDeviceMemoryNative() const
{
   return m_deviceMemory;
}

VkImageCreateFlagBits Image::ImageCreationFlagsToNative(ImageCreationFlags p_flags)
{
   static const Foundation::Std::unordered_map_bootstrap<ImageCreationFlags, VkImageCreateFlagBits> ImageCreationFlagsToNativeMap =
       {
           {ImageCreationFlags::Alias, VK_IMAGE_CREATE_ALIAS_BIT},
           {ImageCreationFlags::Cube_Or_CubeArray, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT},
           {ImageCreationFlags::Array2D, VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT},
       };

   return Foundation::Util::FlagsToNativeHelper<VkImageCreateFlagBits>(ImageCreationFlagsToNativeMap, p_flags);
}

VkImageUsageFlagBits Image::ImageUsageFlagsToNative(ImageUsageFlags p_flags)
{
   static const Foundation::Std::unordered_map_bootstrap<ImageUsageFlags, VkImageUsageFlagBits> ImageUsageFlagsToNativeMap = {
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

} // namespace Render
