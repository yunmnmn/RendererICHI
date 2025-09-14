#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/Vulkan/PhysicalDevice.h>
#include <GHI/Vulkan/Fence.h>

#include <GHI/Device.h>

namespace Render
{

namespace GHI
{
class CommandBuffer;

namespace Vulkan
{

class Device final : public GHI::Device
{
 private:
   Device() = delete;
   Device(DeviceDescriptor&& p_desc);

   void ReleaseInternal() final;

 public:
   ~Device() final;

 public:
   // Get the Logical Device
   VkDevice GetLogicalDeviceNative() const;

   // Get the index of the QueueFamily that is able to present
   uint32_t GetPresentableFamilyQueueIndex() const;

   // Returns the Device's Family Queue Count
   uint32_t GetQueueFamilyCount() const;

   // Get the Queues
   VkQueue GetGraphicsQueueNative() const;
   VkQueue GetComputeQueueNative() const;
   VkQueue GetTransferQueueNative() const;

   // Get the QueueFamilyIndices
   uint32_t GetGraphicsQueueFamilyIndex() const;
   uint32_t GetCompuateQueueFamilyIndex() const;
   uint32_t GetTransferQueueFamilyIndex() const;

   std::tuple<VkDeviceMemory, uint64_t> AllocateDeviceMemory(VkMemoryRequirements p_memoryRequirements,
                                                               MemoryPropertyFlags p_memoryProperties);

   virtual void QueueSubmitInternal(QueueFamilyType p_executingQueueType, std::span<Ptr<GHI::CommandBuffer>> p_commandBuffers,
                                    std::span<TimelineSemaphoreSubmitInfo> p_waitTimelineSemaphores,
                                    std::span<TimelineSemaphoreSubmitInfo> p_signalTimelineSemaphores) = 0;

   VkDevice GetLogicalDevice() const
   {
      return m_logicalDevice;
   }

 private:
   // Native Logical Device
   VkDevice m_logicalDevice = VK_NULL_HANDLE;

   // Native Physical Device
   VkQueue m_graphicsQueue = VK_NULL_HANDLE;

   // The PhysicalDevice's QueueFamilyProperties
   std::vector<QueueFamily> m_queueFamilyArray;

   // The PhysicalDevice's supported ExtensionProperties
   std::vector<VkExtensionProperties> m_extensionProperties;

   // The QueueFamily index that will be used to present the framebuffer
   uint32_t m_presentQueueFamilyIndex;

   // QueueFamilyHandle -> Queues
   std::unordered_map<QueueFamilyHandle, VkQueue, QueueFamilyHandle> m_queues;

   std::unique_ptr<class AsyncUploadQueue> m_uploadQueue;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
