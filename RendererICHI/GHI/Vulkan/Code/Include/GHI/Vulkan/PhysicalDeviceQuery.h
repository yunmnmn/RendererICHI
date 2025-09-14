#pragma once

#include <span>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

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

 private:
   VkQueueFamilyProperties m_queueFamilyProperties;
   Ptr<class Device> m_Device;
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
   PhysicalDeviceQuery(VkPhysicalDevice p_physicalDevice);

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

   bool SupportPresenting() const;

   bool IsDiscreteGpu() const;

   bool IsIntegratedGpu() const;

 private:
   uint32_t GetPresentingFamilyQueueIndex() const;

   // Get the minimum queue family index depending on the requirements
   QueueFamilyHandle GetSuitedQueueFamilyHandle(VkQueueFlagBits queueFlags);

   // Get the Family queue index that supports presenting
   uint32_t GetSuitedPresentQueueFamilyIndex();

 private:
   // Physical Device properties
   VkPhysicalDeviceProperties m_physicalDeviceProperties = {};

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
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render