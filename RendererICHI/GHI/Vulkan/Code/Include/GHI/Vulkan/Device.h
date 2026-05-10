#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <array>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include <GHI/Vulkan/PhysicalDevice.h>
#include <GHI/Vulkan/PhysicalDeviceQuery.h>

#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

class CommandBuffer;
class Fence;

namespace Vulkan
{

class QueueTimelineTracker;

struct DynamicStateSupport
{
   bool m_extendedDynamicState = false;
   bool m_extendedDynamicState2 = false;
};

class Device final : public GHI::Device
{
   friend struct ResourceFactory;

 public:
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

   VkMemoryRequirements GetImageMemoryRequirements(const VkImageCreateInfo& p_createInfo) const;
   VkMemoryRequirements GetBufferMemoryRequirements(const VkBufferCreateInfo& p_createInfo) const;

   uint32_t GetCompatibleMemoryTypeBits(uint32_t p_typeBits, MemoryPropertyFlags p_memoryProperties) const;

   std::tuple<VkDeviceMemory, uint64_t> AllocateDeviceMemory(VkMemoryRequirements p_memoryRequirements,
                                                             MemoryPropertyFlags p_memoryProperties,
                                                             VkMemoryAllocateFlags p_allocateFlags = 0u);

   QueueSubmitResult QueueSubmitInternal(QueueFamilyType p_executingQueueType,
                                         const std::vector<Ptr<GHI::CommandBuffer>>& p_commandBuffers,
                                         const std::vector<FenceSubmitInfo>& p_waitFence,
                                         const std::vector<FenceSubmitInfo>& p_signalAfter) final;

   void WaitFencesInternal(std::vector<FenceSubmitInfo> p_waitFor) final;

   QueueFamilyInfo GetQueueFamilyInfoInternal(QueueFamilyType p_queueType) const final;

   VkDevice GetLogicalDevice() const
   {
      return m_logicalDevice;
   }

   PFN_vkGetDescriptorSetLayoutSizeEXT GetDescriptorSetLayoutSizeEXT() const;
   PFN_vkGetDescriptorSetLayoutBindingOffsetEXT GetDescriptorSetLayoutBindingOffsetEXT() const;
   PFN_vkGetDescriptorEXT GetDescriptorEXT() const;
   PFN_vkCmdBindDescriptorBuffersEXT CmdBindDescriptorBuffersEXT() const;
   PFN_vkCmdSetDescriptorBufferOffsetsEXT CmdSetDescriptorBufferOffsetsEXT() const;
   PFN_vkCmdDrawMeshTasksEXT CmdDrawMeshTasksEXT() const;
   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& GetDescriptorBufferPropertiesEXT() const;
   bool SupportsMeshShader() const;
   bool SupportsTaskShader() const;
   const DynamicStateSupport& GetDynamicStateSupport() const;
   VkPipelineCache GetPipelineCacheNative() const;

 private:
   void LoadDeviceExtensionFunctions();
   void CreatePipelineCache();
   void SavePipelineCache() const;

 private:
   // Native Logical Device
   VkDevice m_logicalDevice = VK_NULL_HANDLE;

   // QueueFamily handles for graphics, compute, transfer
   QueueFamilyHandle m_graphicsQueueFamilyHandle;
   QueueFamilyHandle m_computeQueueFamilyHandle;
   QueueFamilyHandle m_transferQueueFamilyHandle;

   // The PhysicalDevice's QueueFamilyProperties
   std::vector<QueueFamily> m_queueFamilyArray;

   // The PhysicalDevice's supported ExtensionProperties
   std::vector<VkExtensionProperties> m_extensionProperties;

   // The QueueFamily index that will be used to present the framebuffer
   uint32_t m_presentQueueFamilyIndex = static_cast<uint32_t>(-1);

   // QueueFamilyHandle -> Queues
   std::unordered_map<QueueFamilyHandle, VkQueue> m_queues;

   // Physical device features and memory properties
   VkPhysicalDeviceFeatures2 m_deviceFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
   VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties = {};
   VkPhysicalDeviceDescriptorBufferPropertiesEXT m_descriptorBufferProperties = {
       VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT};

   PFN_vkGetDescriptorSetLayoutSizeEXT m_getDescriptorSetLayoutSizeEXT = nullptr;
   PFN_vkGetDescriptorSetLayoutBindingOffsetEXT m_getDescriptorSetLayoutBindingOffsetEXT = nullptr;
   PFN_vkGetDescriptorEXT m_getDescriptorEXT = nullptr;
   PFN_vkCmdBindDescriptorBuffersEXT m_cmdBindDescriptorBuffersEXT = nullptr;
   PFN_vkCmdSetDescriptorBufferOffsetsEXT m_cmdSetDescriptorBufferOffsetsEXT = nullptr;
   PFN_vkCmdDrawMeshTasksEXT m_cmdDrawMeshTasksEXT = nullptr;
   PFN_vkGetDeviceImageMemoryRequirements m_getDeviceImageMemoryRequirements = nullptr;
   PFN_vkGetDeviceBufferMemoryRequirements m_getDeviceBufferMemoryRequirements = nullptr;
   bool m_meshShaderEnabled = false;
   bool m_taskShaderEnabled = false;
   DynamicStateSupport m_dynamicStateSupport;
   VkPipelineCache m_pipelineCacheNative = VK_NULL_HANDLE;

   std::array<std::shared_ptr<QueueTimelineTracker>, static_cast<size_t>(QueueFamilyType::Count)> m_queueTimelineTrackers;
   std::array<uint64_t, static_cast<size_t>(QueueFamilyType::Count)> m_queueTimelineValues = {};

   std::unique_ptr<class AsyncUploadQueue> m_uploadQueue;
   std::unique_ptr<class CommandPoolManager> m_commandPoolManager;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
