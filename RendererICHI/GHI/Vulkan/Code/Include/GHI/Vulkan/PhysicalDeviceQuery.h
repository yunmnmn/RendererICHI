#pragma once

#include <span>
#include <inttypes.h>
#include <stdbool.h>

#include <Vulkan/vulkan.hpp>

#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Device;

// ----------- QueueFamilyHandle -----------

struct QueueFamilyHandle
{
   static constexpr uint32_t InvalidQueueFamilyIndex = static_cast<uint32_t>(-1);

 public:
   QueueFamilyHandle() = default;
   QueueFamilyHandle(uint32_t m_queueFamilyIndex, uint32_t m_queueIndex);

   bool operator==(const QueueFamilyHandle& p_other) const;
   size_t operator()(const QueueFamilyHandle& p_handle) const;

 public:
   bool IsValid() const;

   uint32_t GetQueueFamilyIndex() const;
   uint32_t GetQueueIndex() const;

 private:
   uint64_t CalculateHash() const;

 private:
   // QueueFamily index of the physical device
   uint32_t m_queueFamilyIndex = InvalidQueueFamilyIndex;
   // Queue index within that specific QueueFamily
   uint32_t m_queueIndex = InvalidQueueFamilyIndex;
};

// ----------- QueueFamily -----------

// Helper class to store QueueFamily members
struct QueueFamily
{
 public:
   QueueFamily(VkQueueFamilyProperties p_queueFamilyProperties, uint32_t p_queueFamilyIndex);

   // Checks if the QueueFamily supports the provided flags
   bool SupportFlags(VkQueueFlags queueFlags) const;

   // Returns the number of queues in the QueueFamily
   uint32_t GetQueueCount() const;

   // Returns the number of queues allocated in the QueueFamily
   uint32_t GetAllocatedQueueCount() const;

   // Add count to the queue
   QueueFamilyHandle CreateQueueFamilyHandle();

   // Checks if there are any more available queues left in the family
   bool AvailableQueue() const;

   // Returns the number of supported queues in this QueueFamily (graphics, compute, etc);
   uint32_t GetSupportedQueuesCount() const;
   QueueTypeFlags GetSupportedQueueTypeFlags() const;

 private:
   VkQueueFamilyProperties m_queueFamilyProperties;
   Ptr<Device> m_Device;
   uint32_t m_queueFamilyIndex = 0u;

   uint32_t m_allocatedQueueCount = 0u;
};

// ----------- SurfaceQuery -----------

struct SurfaceQuery
{
 public:
   SurfaceQuery() = default;
   SurfaceQuery(VkPhysicalDevice p_device, VkSurfaceKHR p_surface);

   const VkSurfaceCapabilitiesKHR& GetSurfaceCapabilities() const;

   std::span<const VkSurfaceFormatKHR> GetSupportedFormats() const;

   std::span<const VkPresentModeKHR> GetSupportedPresentModes() const;

   bool SupportSwapchain() const;

 private:
   // TODO: remove capabilities?
   VkSurfaceCapabilitiesKHR m_capabilities;
   std::vector<VkSurfaceFormatKHR> m_formats;
   std::vector<VkPresentModeKHR> m_presentModes;
};

// ----------- PhysicalDeviceQuery -----------

class PhysicalDeviceQuery
{
   static constexpr uint32_t InvalidQueueFamilyIndex = static_cast<uint32_t>(-1);

 public:
   PhysicalDeviceQuery(VkInstance p_instance, VkPhysicalDevice p_physicalDevice);

   ~PhysicalDeviceQuery() = default;

 public:
   // Check whether the DeviceExtension is supported on this device
   bool IsDeviceExtensionSupported(std::string_view p_deviceExtension) const;

   // Get the index of the QueueFamily that is able to present
   uint32_t GetPresentableFamilyQueueIndex() const;

   // Returns the Device's Family Queue Count
   uint32_t GetQueueFamilyCount() const;

   QueueFamilyHandle GetGraphicsQueueFamilyHandle() const;
   QueueFamilyHandle GetComputeQueueFamilyHandle() const;
   QueueFamilyHandle GetTransferQueueFamilyHandle() const;
   QueueFamilyInfo GetQueueFamilyInfo(QueueFamilyType p_queueType) const;

   const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const;
   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& GetDescriptorBufferPropertiesEXT() const;

   QueueTypeFlags GetQueueTypeFlags() const;
   PhysicalDeviceFeatureFlags GetPhysicalDeviceFeatureFlags() const;
   GPUType GetGPUTypes() const;

   bool SupportPresenting() const;

   bool IsDiscreteGpu() const;

   bool IsIntegratedGpu() const;

   bool IsViable() const;

 private:
   void QueryPhysicalDeviceFeatures();
   void QuerySupportedQueues();
   void QuerySupportedFeatures();
   void QueryGpuType();

   uint32_t SupportQueueFamilyFlags(VkQueueFlags queueFlags) const;

   uint32_t GetPresentingFamilyQueueIndex() const;

   // Get the minimum queue family index depending on the requirements
   QueueFamilyHandle GetSuitedQueueFamilyHandle(VkQueueFlagBits queueFlags);

   // Get the Family queue index that supports presenting
   uint32_t GetSuitedPresentQueueFamilyIndex() const;

 private:
   // Physical Device properties
   VkPhysicalDeviceProperties m_physicalDeviceProperties = {};
   VkPhysicalDeviceDescriptorBufferPropertiesEXT m_descriptorBufferProperties = {
       VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT};

   // PHysical Device Memory properties
   VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties = {};

   // The PhysicalDevice's QueueFamilyProperties
   std::vector<QueueFamily> m_queueFamilyArray;

   // The PhysicalDevice's supported ExtensionProperties
   std::vector<VkExtensionProperties> m_extensionProperties;

   uint32_t m_presentQueueFamilyHandle = InvalidQueueFamilyIndex;

   QueueFamilyHandle m_graphicsQueueFamilyHandle;
   QueueFamilyHandle m_computeQueueFamilyHandle;
   QueueFamilyHandle m_transferQueueFamilyHandle;

   QueueTypeFlags m_supportedQueues = {};
   PhysicalDeviceFeatureFlags m_supportedFeatures = {};
   GPUType m_type = GPUType::Invalid;

   VkPhysicalDevice m_physicalDevice;

   VkInstance m_instance;

   VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputDynamicState = {};
   VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
   VkPhysicalDeviceColorWriteEnableFeaturesEXT colorWriteCreateInfo = {};
   VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT dynamicState1 = {};
   VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicState = {};
   VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamicState2 = {};
   VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
   VkPhysicalDeviceVulkan12Features supportedVulkan12Features = {};
   VkPhysicalDeviceVulkan13Features supportedVulkan13Features = {};
   VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutableDescriptorType = {};
   VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures = {};
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render

namespace std
{
template <>
struct hash<Render::GHI::Vulkan::QueueFamilyHandle>
{
   size_t operator()(const Render::GHI::Vulkan::QueueFamilyHandle& p_handle) const noexcept
   {
      const uint64_t family = p_handle.GetQueueFamilyIndex();
      const uint64_t queue = p_handle.GetQueueIndex();
      return static_cast<size_t>((family << 32u) ^ queue);
   }
};
} // namespace std
