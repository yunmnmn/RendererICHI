#include <GHI/Vulkan/Device.h>

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include <Util/Macro.h>
#include <Util/Assert.h>
#include <Util/MurmurHash3.h>

#include <GHI/Vulkan/CommandBuffer.h>
#include <GHI/Vulkan/Fence.h>
#include <GHI/Vulkan/PhysicalDevice.h>
#include <GHI/Vulkan/VulkanInstance.h>
#include <GHI/Vulkan/RendererTypes.h>
#include <GHI/Vulkan/AsyncUploadQueue.h>
#include <GHI/Vulkan/CommandPoolManager.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class QueueTimelineTracker final : public GHI::SubmissionTracker
{
 public:
   explicit QueueTimelineTracker(VkDevice p_device) : m_device(p_device)
   {
      VkSemaphoreTypeCreateInfo typeCreateInfo = {};
      typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
      typeCreateInfo.pNext = nullptr;
      typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
      typeCreateInfo.initialValue = 0u;

      VkSemaphoreCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      createInfo.pNext = &typeCreateInfo;
      createInfo.flags = {};

      const VkResult res = vkCreateSemaphore(m_device, &createInfo, nullptr, &m_semaphoreNative);
      ASSERT(res == VK_SUCCESS, "Failed to create a queue timeline tracker semaphore");
   }

   ~QueueTimelineTracker() final
   {
      if (m_semaphoreNative != VK_NULL_HANDLE)
      {
         vkDestroySemaphore(m_device, m_semaphoreNative, nullptr);
      }
   }

   bool IsValueSignaled(uint64_t p_value) const final
   {
      uint64_t currentValue = 0u;
      const VkResult res = vkGetSemaphoreCounterValue(m_device, m_semaphoreNative, &currentValue);
      ASSERT(res == VK_SUCCESS, "Failed to get the queue timeline tracker counter value");
      return currentValue >= p_value;
   }

   VkSemaphore GetSemaphoreNative() const
   {
      return m_semaphoreNative;
   }

 private:
   VkDevice m_device = VK_NULL_HANDLE;
   VkSemaphore m_semaphoreNative = VK_NULL_HANDLE;
};

// ----------- Device -----------

Device::Device(DeviceDescriptor&& p_desc) : GHI::Device(std::move(p_desc))
{
   // Initialize queue family handles from the physical device
   {
      auto physDevice = Cast<Vulkan::PhysicalDevice>(GetPhysicalDevice());
      m_graphicsQueueFamilyHandle = physDevice->GetGraphicsQueueFamilyHandle();
      m_computeQueueFamilyHandle = physDevice->GetComputeQueueFamilyHandle();
      m_transferQueueFamilyHandle = physDevice->GetTransferQueueFamilyHandle();
   }

   static const auto CreateQueueCreateInfoFromHandle = [](std::vector<QueueFamilyHandle>&& p_handles,
                                                          std::vector<VkDeviceQueueCreateInfo>& p_createInfos) {
      const uint32_t MaxQueuePerFamily = 6u;
      static const float priority[MaxQueuePerFamily] = {0.0f};

      std::unordered_map<QueueFamilyHandle, VkDeviceQueueCreateInfo> handleToCreateInfo;

      // Create the device QueueInfos
      for (const auto& handle : p_handles)
      {
         const auto& mapIt = handleToCreateInfo.find(handle);
         if (mapIt != handleToCreateInfo.end())
         {
            // If it exists, add a count
            mapIt->second.queueCount++;
         }
         else
         {
            // If it doesn't exist, create a new CreateInfo
            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = handle.GetQueueFamilyIndex();
            queueInfo.queueCount = 1u;
            queueInfo.pQueuePriorities = priority;
            handleToCreateInfo[handle] = queueInfo;
         }
      }

      // Add the CreateInfos to the array
      p_createInfos.reserve(p_createInfos.size());
      for (auto& createInfo : handleToCreateInfo)
      {
         p_createInfos.push_back(std::move(createInfo.second));
      }
   };

   // Create all QueueCreateInfos
   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   CreateQueueCreateInfoFromHandle({m_graphicsQueueFamilyHandle, m_computeQueueFamilyHandle, m_transferQueueFamilyHandle},
                                   queueCreateInfos);

   std::vector<const char*> deviceExtensions;
   if (Cast<Vulkan::PhysicalDevice>(GetPhysicalDevice())->IsDeviceExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
   {
      deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
   }

   if (VulkanInstance::Get()->IsDebugEnabled())
   {
      if (Cast<Vulkan::PhysicalDevice>(GetPhysicalDevice())->IsDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
      {
         deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }
   }

   ASSERT(Cast<Vulkan::PhysicalDevice>(GetPhysicalDevice())->IsDeviceExtensionSupported(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME),
          "Device does not support VK_EXT_descriptor_buffer");
   deviceExtensions.push_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

   VkPhysicalDeviceVulkan12Features vulkan12Features = {};
   vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
   vulkan12Features.pNext = nullptr;
   vulkan12Features.bufferDeviceAddress = VK_TRUE;
   vulkan12Features.timelineSemaphore = VK_TRUE;

   VkPhysicalDeviceVulkan13Features vulkan13Features = {};
   vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
   vulkan13Features.pNext = &vulkan12Features;
   vulkan13Features.dynamicRendering = VK_TRUE;
   vulkan13Features.synchronization2 = VK_TRUE;

   VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures = {};
   descriptorBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
   descriptorBufferFeatures.pNext = &vulkan13Features;
   descriptorBufferFeatures.descriptorBuffer = VK_TRUE;
   descriptorBufferFeatures.descriptorBufferPushDescriptors = VK_TRUE;

   m_deviceFeatures.pNext = &descriptorBufferFeatures;

   // Create the Logical Device Resource
   {
      VkDeviceCreateInfo deviceCreateInfo = {};
      deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      deviceCreateInfo.pNext = &m_deviceFeatures;
      deviceCreateInfo.flags = {}; // No flags as of yet
      deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
      deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
      deviceCreateInfo.enabledLayerCount = 0;   // Deprecated
      deviceCreateInfo.ppEnabledLayerNames = 0; // Deprecated
      deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
      deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
      deviceCreateInfo.pEnabledFeatures = nullptr; // Must be null when pNext is provided

      const VkResult result =
          vkCreateDevice(Cast<Vulkan::PhysicalDevice>(GetPhysicalDevice())->GetPhysicalDeviceNative(), &deviceCreateInfo, nullptr,
                         &m_logicalDevice);
      ASSERT(result == VK_SUCCESS, "Failed to create a logical device");
   }

   LoadDeviceExtensionFunctions();

   for (std::shared_ptr<QueueTimelineTracker>& tracker : m_queueTimelineTrackers)
   {
      tracker = std::make_shared<QueueTimelineTracker>(m_logicalDevice);
   }

   {
      const Ptr<Vulkan::PhysicalDevice> physicalDevice = Cast<Vulkan::PhysicalDevice>(GetPhysicalDevice());
      m_deviceMemoryProperties = physicalDevice->GetMemoryProperties();
      m_descriptorBufferProperties = physicalDevice->GetDescriptorBufferPropertiesEXT();
   }

   // Get the queues from the Logical Device
   {
      const auto GetQueueFromDevice = [this](const QueueFamilyHandle& p_handle) {
         const auto& queueIt = m_queues.find(p_handle);
         if (queueIt == m_queues.end())
         {
            vkGetDeviceQueue(m_logicalDevice, p_handle.GetQueueFamilyIndex(), p_handle.GetQueueIndex(), &m_queues[p_handle]);
         }
      };

      // Get the GraphicsQueue:
      GetQueueFromDevice(m_graphicsQueueFamilyHandle);

      // Get the Compute Queue
      GetQueueFromDevice(m_computeQueueFamilyHandle);

      // Get the Transfer Queue
      GetQueueFromDevice(m_transferQueueFamilyHandle);
   }
}

Device::~Device()
{
}

void Device::ReleaseInternal()
{
   [[maybe_unused]] const VkResult waitRes = vkDeviceWaitIdle(m_logicalDevice);
   ASSERT(waitRes == VK_SUCCESS, "Failed to wait for the device to become idle");

   ClearSubmittedCommandBufferBatches();

   // Destroy subsystems while the logical device is still valid, breaking the circular Ptr<Device> reference
   if (m_uploadQueue)
   {
      GHI::AsyncUploadQueueInterface::Unregister();
      m_uploadQueue.reset();
   }
   if (m_commandPoolManager)
   {
      CommandPoolManagerInterface::Unregister();
      m_commandPoolManager.reset();
   }

   for (std::shared_ptr<QueueTimelineTracker>& tracker : m_queueTimelineTrackers)
   {
      tracker.reset();
   }

   vkDestroyDevice(m_logicalDevice, nullptr);
}

VkDevice Device::GetLogicalDeviceNative() const
{
   return m_logicalDevice;
}

void Device::LoadDeviceExtensionFunctions()
{
   m_getDescriptorSetLayoutSizeEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(
       vkGetDeviceProcAddr(m_logicalDevice, "vkGetDescriptorSetLayoutSizeEXT"));
   ASSERT(m_getDescriptorSetLayoutSizeEXT, "Failed to load vkGetDescriptorSetLayoutSizeEXT");

   m_getDescriptorSetLayoutBindingOffsetEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutBindingOffsetEXT>(
       vkGetDeviceProcAddr(m_logicalDevice, "vkGetDescriptorSetLayoutBindingOffsetEXT"));
   ASSERT(m_getDescriptorSetLayoutBindingOffsetEXT, "Failed to load vkGetDescriptorSetLayoutBindingOffsetEXT");

   m_getDescriptorEXT = reinterpret_cast<PFN_vkGetDescriptorEXT>(vkGetDeviceProcAddr(m_logicalDevice, "vkGetDescriptorEXT"));
   ASSERT(m_getDescriptorEXT, "Failed to load vkGetDescriptorEXT");

   m_cmdBindDescriptorBuffersEXT = reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(
       vkGetDeviceProcAddr(m_logicalDevice, "vkCmdBindDescriptorBuffersEXT"));
   ASSERT(m_cmdBindDescriptorBuffersEXT, "Failed to load vkCmdBindDescriptorBuffersEXT");

   m_cmdSetDescriptorBufferOffsetsEXT = reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(
       vkGetDeviceProcAddr(m_logicalDevice, "vkCmdSetDescriptorBufferOffsetsEXT"));
   ASSERT(m_cmdSetDescriptorBufferOffsetsEXT, "Failed to load vkCmdSetDescriptorBufferOffsetsEXT");

   m_getDeviceImageMemoryRequirements = reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirements>(
       vkGetDeviceProcAddr(m_logicalDevice, "vkGetDeviceImageMemoryRequirements"));
   if (!m_getDeviceImageMemoryRequirements)
   {
      m_getDeviceImageMemoryRequirements = reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirements>(
          vkGetDeviceProcAddr(m_logicalDevice, "vkGetDeviceImageMemoryRequirementsKHR"));
   }
   ASSERT(m_getDeviceImageMemoryRequirements, "Failed to load vkGetDeviceImageMemoryRequirements");

   m_getDeviceBufferMemoryRequirements = reinterpret_cast<PFN_vkGetDeviceBufferMemoryRequirements>(
       vkGetDeviceProcAddr(m_logicalDevice, "vkGetDeviceBufferMemoryRequirements"));
   if (!m_getDeviceBufferMemoryRequirements)
   {
      m_getDeviceBufferMemoryRequirements = reinterpret_cast<PFN_vkGetDeviceBufferMemoryRequirements>(
          vkGetDeviceProcAddr(m_logicalDevice, "vkGetDeviceBufferMemoryRequirementsKHR"));
   }
   ASSERT(m_getDeviceBufferMemoryRequirements, "Failed to load vkGetDeviceBufferMemoryRequirements");
}

PFN_vkGetDescriptorSetLayoutSizeEXT Device::GetDescriptorSetLayoutSizeEXT() const
{
   return m_getDescriptorSetLayoutSizeEXT;
}

PFN_vkGetDescriptorSetLayoutBindingOffsetEXT Device::GetDescriptorSetLayoutBindingOffsetEXT() const
{
   return m_getDescriptorSetLayoutBindingOffsetEXT;
}

PFN_vkGetDescriptorEXT Device::GetDescriptorEXT() const
{
   return m_getDescriptorEXT;
}

PFN_vkCmdBindDescriptorBuffersEXT Device::CmdBindDescriptorBuffersEXT() const
{
   return m_cmdBindDescriptorBuffersEXT;
}

PFN_vkCmdSetDescriptorBufferOffsetsEXT Device::CmdSetDescriptorBufferOffsetsEXT() const
{
   return m_cmdSetDescriptorBufferOffsetsEXT;
}

const VkPhysicalDeviceDescriptorBufferPropertiesEXT& Device::GetDescriptorBufferPropertiesEXT() const
{
   return m_descriptorBufferProperties;
}

VkQueue Device::GetGraphicsQueueNative() const
{
   const auto& queueIt = m_queues.find(m_graphicsQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Grahpics Queue doesn't exist");

   return queueIt->second;
}

VkMemoryRequirements Device::GetImageMemoryRequirements(const VkImageCreateInfo& p_createInfo) const
{
   ASSERT(m_getDeviceImageMemoryRequirements != nullptr, "vkGetDeviceImageMemoryRequirements is not loaded");

   VkDeviceImageMemoryRequirements deviceRequirements = {};
   deviceRequirements.sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS;
   deviceRequirements.pCreateInfo = &p_createInfo;
   deviceRequirements.planeAspect = static_cast<VkImageAspectFlagBits>(0u);

   VkMemoryRequirements2 memoryRequirements = {};
   memoryRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
   m_getDeviceImageMemoryRequirements(m_logicalDevice, &deviceRequirements, &memoryRequirements);

   return memoryRequirements.memoryRequirements;
}

VkMemoryRequirements Device::GetBufferMemoryRequirements(const VkBufferCreateInfo& p_createInfo) const
{
   ASSERT(m_getDeviceBufferMemoryRequirements != nullptr, "vkGetDeviceBufferMemoryRequirements is not loaded");

   VkDeviceBufferMemoryRequirements deviceRequirements = {};
   deviceRequirements.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS;
   deviceRequirements.pCreateInfo = &p_createInfo;

   VkMemoryRequirements2 memoryRequirements = {};
   memoryRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
   m_getDeviceBufferMemoryRequirements(m_logicalDevice, &deviceRequirements, &memoryRequirements);

   return memoryRequirements.memoryRequirements;
}

uint32_t Device::GetCompatibleMemoryTypeBits(uint32_t p_typeBits, MemoryPropertyFlags p_memoryProperties) const
{
   const VkMemoryPropertyFlags memoryPropertyFlagsNative =
       RenderTypeToNative::MemoryPropertyFlagsToNative(p_memoryProperties);

   uint32_t compatibleTypeBits = 0u;
   for (uint32_t i = 0; i < m_deviceMemoryProperties.memoryTypeCount; i++)
   {
      if (((p_typeBits >> i) & 1u) == 0u)
      {
         continue;
      }

      if ((m_deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlagsNative) ==
          memoryPropertyFlagsNative)
      {
         compatibleTypeBits |= 1u << i;
      }
   }

   return compatibleTypeBits;
}

std::tuple<VkDeviceMemory, uint64_t> Device::AllocateDeviceMemory(VkMemoryRequirements p_memoryRequirements,
                                                                  MemoryPropertyFlags p_memoryProperties,
                                                                  VkMemoryAllocateFlags p_allocateFlags)
{
   const auto GetMemoryTypeIndex = [this](uint32_t p_typeBits, MemoryPropertyFlags p_memoryProperties) -> uint32_t {
      const uint32_t compatibleTypeBits = GetCompatibleMemoryTypeBits(p_typeBits, p_memoryProperties);
      for (uint32_t i = 0; i < m_deviceMemoryProperties.memoryTypeCount; i++)
      {
         if (((compatibleTypeBits >> i) & 1u) == 1u)
         {
            return i;
         }
      }

      ASSERT(false, "Can't find a index into the DeviceMemoryProperties which support these combinations of memory properties");
      return static_cast<uint32_t>(-1);
   };

   VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
   uint64_t allocatedSize = 0u;
   {
      VkMemoryAllocateFlagsInfo allocateFlagsInfo = {};
      allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
      allocateFlagsInfo.flags = p_allocateFlags;

      // Allocate the memory
      VkMemoryAllocateInfo memoryAllocateInfo = {};
      memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memoryAllocateInfo.pNext = p_allocateFlags == 0u ? nullptr : &allocateFlagsInfo;
      memoryAllocateInfo.allocationSize = p_memoryRequirements.size;
      memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(p_memoryRequirements.memoryTypeBits, p_memoryProperties);
      [[maybe_unused]] const VkResult res = vkAllocateMemory(GetLogicalDeviceNative(), &memoryAllocateInfo, nullptr, &deviceMemory);

      ASSERT(res == VK_SUCCESS, "Failed to allocate the device memory for the buffer");

      allocatedSize = p_memoryRequirements.size;
   }

   return {deviceMemory, allocatedSize};
}

QueueSubmitResult Device::QueueSubmitInternal(QueueFamilyType p_executingQueueType,
                                              const std::vector<Ptr<GHI::CommandBuffer>>& p_commandBuffers,
                                              const std::vector<FenceSubmitInfo>& p_waitFence,
                                              const std::vector<FenceSubmitInfo>& p_signalAfter)
{
   const bool trackSubmission = !p_commandBuffers.empty();
   size_t queueIndex = static_cast<size_t>(p_executingQueueType);
   uint64_t submitValue = 0u;
   std::shared_ptr<QueueTimelineTracker> submitTracker;

   if (trackSubmission)
   {
      ASSERT(queueIndex < m_queueTimelineTrackers.size(), "Invalid queue family type");
      ASSERT(m_queueTimelineTrackers[queueIndex] != nullptr, "Queue timeline tracker was not initialized");

      submitValue = ++m_queueTimelineValues[queueIndex];
      submitTracker = m_queueTimelineTrackers[queueIndex];
   }

   std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
   waitSemaphores.reserve(p_waitFence.size());
   {
      for (const FenceSubmitInfo& fence : p_waitFence)
      {
         Ptr<Vulkan::Fence> vulkanFence = Cast<Vulkan::Fence>(fence.m_fence);
         const uint64_t semaphoreValue = vulkanFence->IsTimelineSemaphore() ? fence.m_value : 0u;

         // NOTE: VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT is not ideal, but DX12 doesn't support it, so we'll have to support the least
         // common denominator
         waitSemaphores.emplace_back(VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr,
                                     vulkanFence->GetSemaphoreNative(), semaphoreValue,
                                     VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0u);
      }
   }

   std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
   signalSemaphores.reserve(p_signalAfter.size() + (trackSubmission ? 1u : 0u));
   {
      for (const FenceSubmitInfo& fence : p_signalAfter)
      {
         Ptr<Vulkan::Fence> vulkanFence = Cast<Vulkan::Fence>(fence.m_fence);
         const uint64_t semaphoreValue = vulkanFence->IsTimelineSemaphore() ? fence.m_value : 0u;

         // NOTE: VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT is not ideal, but DX12 doesn't support it, so we'll have to support the least
         // common denominator
         signalSemaphores.emplace_back(VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr,
                                       vulkanFence->GetSemaphoreNative(), semaphoreValue,
                                       VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0u);
      }

      if (trackSubmission)
      {
         signalSemaphores.emplace_back(VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, submitTracker->GetSemaphoreNative(), submitValue,
                                       VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0u);
      }
   }

   std::vector<VkCommandBufferSubmitInfo> commandBufferSubmits;
   commandBufferSubmits.reserve(p_commandBuffers.size());
   for (Ptr<GHI::CommandBuffer> commandBuffer : p_commandBuffers)
   {
      commandBufferSubmits.emplace_back(VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, nullptr,
                                        Cast<Vulkan::CommandBuffer>(commandBuffer)->GetCommandBufferNative(), 0u);
   }

   VkQueue queue = {};
   switch (p_executingQueueType)
   {
   case QueueFamilyType::GraphicsQueue:
      queue = GetGraphicsQueueNative();
      break;
   case QueueFamilyType::ComputeQueue:
      queue = GetComputeQueueNative();
      break;
   case QueueFamilyType::TransferQueue:
      queue = GetTransferQueueNative();
      break;
   }

   VkSubmitInfo2 submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                            .pNext = nullptr,
                            .flags = {},
                            .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphores.size()),
                            .pWaitSemaphoreInfos = waitSemaphores.data(),
                            .commandBufferInfoCount = static_cast<uint32_t>(commandBufferSubmits.size()),
                            .pCommandBufferInfos = commandBufferSubmits.data(),
                            .signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphores.size()),
                            .pSignalSemaphoreInfos = signalSemaphores.data()};

   const VkResult res = vkQueueSubmit2(queue, 1u, &submitInfo, VK_NULL_HANDLE);
   ASSERT(res == VK_SUCCESS, "Failed to submit the queue");

   return QueueSubmitResult{.m_tracker = std::move(submitTracker), .m_value = submitValue};
}

VkQueue Device::GetComputeQueueNative() const
{
   const auto& queueIt = m_queues.find(m_computeQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Compute Queue doesn't exist");

   return queueIt->second;
}

VkQueue Device::GetTransferQueueNative() const
{
   const auto& queueIt = m_queues.find(m_transferQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Transfer Queue doesn't exist");

   return queueIt->second;
}

uint32_t Device::GetGraphicsQueueFamilyIndex() const
{
   return m_graphicsQueueFamilyHandle.GetQueueFamilyIndex();
}

uint32_t Device::GetCompuateQueueFamilyIndex() const
{
   return m_computeQueueFamilyHandle.GetQueueFamilyIndex();
}

uint32_t Device::GetTransferQueueFamilyIndex() const
{
   return m_transferQueueFamilyHandle.GetQueueFamilyIndex();
}

QueueFamilyInfo Device::GetQueueFamilyInfoInternal(QueueFamilyType p_queueType) const
{
   return Cast<Vulkan::PhysicalDevice>(GetPhysicalDevice())->GetQueueFamilyInfo(p_queueType);
}

void Device::WaitFencesInternal(std::vector<FenceSubmitInfo> p_waitFor)
{
   for (const FenceSubmitInfo& fenceInfo : p_waitFor)
   {
      fenceInfo.m_fence->WaitForValue(fenceInfo.m_value);
   }
}

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
