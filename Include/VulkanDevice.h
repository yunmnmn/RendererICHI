#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <std/vector.h>

#include <glad/vulkan.h>

namespace Render
{
struct VulkanDeviceDescriptor
{
   VkPhysicalDevice m_physicalDevice;
};

class VulkanDevice : public RenderResource<VulkanDevice, VulkanDeviceDescriptor>
{
   static constexpr uint32_t InvalidQueueFamilyIndex = static_cast<uint32_t>(-1);

 public:
   // NOTE: Only support 12 devices per instance
   static constexpr size_t MaxDeviceCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VulkanDevice, 1u, static_cast<uint32_t>(sizeof(VulkanDevice) * MaxDeviceCount));

   VulkanDevice(VulkanDeviceDescriptor&& p_desc);
   ~VulkanDevice();

   // Get the minimum queue family index depending on the requirements
   uint32_t GetSuitedFamilyQueueIndex(VkQueueFlagBits queueFlags) const;

   // Check whether the DeviceExtension is supported on this device
   bool IsDeviceExtensionSupported(const char* p_deviceExtension) const;

   // Create the logical device
   void CreateLogicalDevice(Render::vector<const char*>&& p_deviceExtensions);

   VkPhysicalDevice GetPhysicalDevice() const;

   VkDevice GetLogicalDevice() const;

   uint32_t GetPresentableFamilyQueueIndex() const;

 private:
   VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
   VkDevice m_logicalDevice = VK_NULL_HANDLE;
   VkCommandPool m_commandPool = VK_NULL_HANDLE;
   VkQueue m_graphicsQueue = VK_NULL_HANDLE;
   VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

   VkPhysicalDeviceProperties m_physicalDeviceProperties = {};

   // Get the device specific features
   VkPhysicalDeviceVulkan12Features m_supportedVulkan12Features = {};
   VkPhysicalDeviceFeatures2 m_deviceFeatures = {};

   VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties = {};

   // The PhysicalDevice's QueueFamilyProperties
   Render::vector<VkQueueFamilyProperties> m_queueFamilyProperties;

   // The PhysicalDevice's supported ExtensionProperties
   Render::vector<VkExtensionProperties> m_extensionProperties;
   Render::vector<Foundation::Util::HashName> m_enabledDeviceExtensions;
   uint32_t m_presentFamilyQueueIndex = InvalidQueueFamilyIndex;
};

} // namespace Render
