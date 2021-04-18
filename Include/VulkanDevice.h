#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <std/vector.h>

#include <EASTL/span.h>

namespace Render
{
class VulkanInstance;
class RenderWindow;

struct VulkanDeviceDescriptor
{
   VkPhysicalDevice m_physicalDevice;
   ResourceRef<VulkanInstance> m_vulkanInstance;
};

class VulkanDevice : public RenderResource<VulkanDevice, VulkanDeviceDescriptor>
{
   static constexpr uint32_t InvalidQueueFamilyIndex = static_cast<uint32_t>(-1);

   // Helper class to store QueueFamily members
   struct QueueFamily
   {
    public:
      QueueFamily(VkQueueFamilyProperties p_queueFamilyProperties);

      // Checks if the QueueFamily supports the provided flags
      bool SupportFlags(VkQueueFlags queueFlags) const;

      // Returns the number of queues supported in this QueueFamily
      uint32_t GetQueueCount() const;

    private:
      VkQueueFamilyProperties m_queueFamilyProperties;
      ResourceRef<VulkanDevice> m_vulkanDevice;
   };

   // Helper class to store Swapchain Details
   struct SwapchainSupportDetails
   {
    public:
      SwapchainSupportDetails() = default;
      SwapchainSupportDetails(ResourceRef<VulkanDevice> p_device, ResourceRef<RenderWindow> p_window);

      void GetSurfaceCapabilities() const;
      eastl::span<VkSurfaceFormatKHR> GetSupportedFormats();
      eastl::span<VkPresentModeKHR> GetSupportedPresentModes();

    private:
      // TODO: remove capabilities?
      VkSurfaceCapabilitiesKHR m_capabilities;
      Render::vector<VkSurfaceFormatKHR> m_formats;
      Render::vector<VkPresentModeKHR> m_presentModes;

      ResourceRef<VulkanDevice> m_device;
      ResourceRef<RenderWindow> m_window;
   };

 public:
   // NOTE: Only support 12 devices per instance
   static constexpr size_t MaxDeviceCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VulkanDevice, 1u, static_cast<uint32_t>(sizeof(VulkanDevice) * MaxDeviceCount));

   VulkanDevice(VulkanDeviceDescriptor&& p_desc);
   ~VulkanDevice();

   // Create the logical device
   void CreateLogicalDevice(Render::vector<const char*>&& p_deviceExtensions);

   // Check whether the DeviceExtension is supported on this device
   bool IsDeviceExtensionSupported(const char* p_deviceExtension) const;

   // Returns the first index of the QueueFamily which supports the provided flags
   uint32_t SupportQueueFamilyFlags(VkQueueFlags queueFlags) const;

   // Returns the first index of the QueueFamily which supports Presenting
   uint32_t SupportPresenting() const;

   // Returns the first index of the QueueFamily which supports Presenting
   bool SupportSwapchain();

   // Returns whether the Device is a discrete GPU
   bool IsDiscreteGpu() const;

   // Get the PhysicalDevice
   VkPhysicalDevice GetPhysicalDeviceNative() const;

   // Get the Logical Device
   VkDevice GetLogicalDeviceNative() const;

   // Get the index of the QueueFamily that is able to present
   uint32_t GetPresentableFamilyQueueIndex() const;

   // Returns the Device's Family Queue Count
   uint32_t GetFamilyQueueCount() const;

   // Set the swapchain details of the device depending on the provided window
   void SetSwapchainDetails(ResourceRef<RenderWindow> p_window);

 private:
   // Get the minimum queue family index depending on the requirements
   uint32_t GetSuitedQueueFamilyIndex(VkQueueFlagBits queueFlags) const;

   // Get the Family queue index that supports presenting
   uint32_t GetSuitedPresentQueueFamilyIndex();

   VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
   ResourceRef<VulkanInstance> m_vulkanInstance;

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
   Render::vector<QueueFamily> m_queueFamilyArray;

   // The PhysicalDevice's supported ExtensionProperties
   Render::vector<VkExtensionProperties> m_extensionProperties;
   Render::vector<Foundation::Util::HashName> m_enabledDeviceExtensions;

   // QueueFamilyIndices
   uint32_t m_presentQueueFamilyIndex = InvalidQueueFamilyIndex;
   uint32_t m_graphicsQueueFamilyIndex = InvalidQueueFamilyIndex;

   SwapchainSupportDetails m_swapchainDetails;
};

} // namespace Render
