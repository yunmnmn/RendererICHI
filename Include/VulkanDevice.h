#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <EASTL/span.h>

#include <Std/vector.h>
#include <Std/unordered_map.h>

#include <Memory/AllocatorClass.h>
#include <ResourceReference.h>
#include <RendererTypes.h>
#include <Util/HashName.h>

using namespace Foundation;

namespace Render
{

class VulkanInstance;
class RenderWindow;
class Buffer;
class Surface;

enum class CommandQueueTypes : uint32_t
{
   Graphics = 0u,
   Compute = 1u,
   Transfer = 2u,

   Count,
   Invalid
};

struct VulkanDeviceDescriptor
{
   ResourceRef<VulkanInstance> m_vulkanInstanceRef;
   uint32_t m_physicalDeviceIndex = static_cast<uint32_t>(-1);
   Surface* m_surface = nullptr;
};

class VulkanDevice : public RenderResource<VulkanDevice>
{
   friend class VulkanInstance;

   static constexpr uint32_t InvalidQueueFamilyIndex = static_cast<uint32_t>(-1);

   struct QueueFamilyHandle
   {
      // QueueFamily index of the physical device
      uint32_t m_queueFamilyIndex = InvalidQueueFamilyIndex;
      // Queue index within that specific QueueFamily
      uint32_t m_queueIndex = InvalidQueueFamilyIndex;

      bool operator==(const QueueFamilyHandle& p_other) const;
      size_t operator()(const QueueFamilyHandle& p_handle) const;

    private:
      uint64_t CalculateHash() const;
   };

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
      ResourceRef<VulkanDevice> m_vulkanDevice;
      uint32_t m_queueFamilyIndex = 0u;

      uint32_t m_allocatedQueueCount = 0u;
   };

   // Helper class to store Swapchain Details
 public:
   struct SurfaceProperties
   {
    public:
      SurfaceProperties() = default;
      SurfaceProperties(const VulkanDevice* p_device, const Surface* p_window);

      const VkSurfaceCapabilitiesKHR& GetSurfaceCapabilities() const;
      eastl::span<const VkSurfaceFormatKHR> GetSupportedFormats() const;
      eastl::span<const VkPresentModeKHR> GetSupportedPresentModes() const;

    private:
      // TODO: remove capabilities?
      VkSurfaceCapabilitiesKHR m_capabilities;
      Std::vector<VkSurfaceFormatKHR> m_formats;
      Std::vector<VkPresentModeKHR> m_presentModes;
   };

 public:
   // NOTE: Only support 12 devices per instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VulkanDevice, 1u);

   VulkanDevice(VulkanDeviceDescriptor&& p_desc);
   ~VulkanDevice();

   // Create the logical device
   void CreateLogicalDevice(Std::vector<const char*>&& p_deviceExtensions);

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
   uint32_t GetQueueFamilyCount() const;

   // Get the Queues
   VkQueue GetGraphicsQueueNative() const;
   VkQueue GetComputQueueNative() const;
   VkQueue GetTransferQueueNative() const;

   // Get the QueueFamilyIndices
   uint32_t GetGraphicsQueueFamilyIndex() const;
   uint32_t GetCompuateQueueFamilyIndex() const;
   uint32_t GetTransferQueueFamilyIndex() const;

   // Returns the SwapchainSupportDetail of this device
   const SurfaceProperties& GetSurfaceProperties() const;

   // TODO: Not sure if this is necessary
   const uint32_t GetPresentQueueFamilyIndex() const;

   eastl::tuple<VkDeviceMemory, uint64_t> AllocateDeviceMemory(VkMemoryRequirements p_memoryRequirements,
                                                               MemoryPropertyFlags p_memoryProperties);

 private:
   // Get the minimum queue family index depending on the requirements
   QueueFamilyHandle GetSuitedQueueFamilyHandle(VkQueueFlagBits queueFlags);

   // Get the Family queue index that supports presenting
   uint32_t GetSuitedPresentQueueFamilyIndex();

   // Create a unique Uuid of the CommandQueue
   // TODO: still not unique across multiple devices though...
   uint64_t CreateQueueUuid(CommandQueueTypes p_commandQueueType);

   // Native Logical Device
   VkDevice m_logicalDevice = VK_NULL_HANDLE;
   // Native Physical Device
   VkQueue m_graphicsQueue = VK_NULL_HANDLE;

   // VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

   // Physical Device properties
   VkPhysicalDeviceProperties m_physicalDeviceProperties = {};

   // PHysical Device Memory properties
   VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties = {};

   // Get the device specific features
   VkPhysicalDeviceColorWriteEnableFeaturesEXT colorWriteCreateInfo = {};
   VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT m_dynamicState1 = {};
   VkPhysicalDeviceExtendedDynamicState2FeaturesEXT m_dynamicState2 = {};
   VkPhysicalDeviceDynamicRenderingFeatures m_dynamicRenderingFeatures = {};
   VkPhysicalDeviceVulkan12Features m_supportedVulkan12Features = {};
   VkPhysicalDeviceFeatures2 m_deviceFeatures = {};

   // The PhysicalDevice's QueueFamilyProperties
   Std::vector<QueueFamily> m_queueFamilyArray;

   // The PhysicalDevice's supported ExtensionProperties
   Std::vector<VkExtensionProperties> m_extensionProperties;
   Std::vector<Foundation::Util::HashName> m_enabledDeviceExtensions;

   // QueueFamilyHandles
   QueueFamilyHandle m_graphicsQueueFamilyHandle;
   QueueFamilyHandle m_computeQueueFamilyHandle;
   QueueFamilyHandle m_transferQueueFamilyHandle;

   // The QueueFamily index that will be used to present the framebuffer
   uint32_t m_presentQueueFamilyIndex;

   // QueueFamilyHandle -> Queues
   Std::unordered_map<QueueFamilyHandle, VkQueue, QueueFamilyHandle> m_queues;

   // Surface properties for the Device
   SurfaceProperties m_surfaceProperties;

   ResourceRef<VulkanInstance> m_vulkanInstanceRef;
   uint32_t m_physicalDeviceIndex = static_cast<uint32_t>(-1);
   VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
};

} // namespace Render
