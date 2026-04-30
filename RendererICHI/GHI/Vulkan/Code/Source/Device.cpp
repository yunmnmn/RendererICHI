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

std::tuple<VkDeviceMemory, uint64_t> Device::AllocateDeviceMemory(VkMemoryRequirements p_memoryRequirements,
                                                                  MemoryPropertyFlags p_memoryProperties)
{
   const auto GetMemoryTypeIndex = [this](uint32_t p_typeBits, MemoryPropertyFlags p_memoryProperties) -> uint32_t {
      VkMemoryPropertyFlags memoryPropertyFlagsNative = RenderTypeToNative::MemoryPropertyFlagsToNative(p_memoryProperties);
      // Iterate over all memory types available for the device used in this example
      for (uint32_t i = 0; i < m_deviceMemoryProperties.memoryTypeCount; i++)
      {
         if (((p_typeBits >> i) & 1u) == 1u)
         {
            if ((m_deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlagsNative) == memoryPropertyFlagsNative)
            {
               return i;
            }
         }
      }

      ASSERT(false, "Can't find a index into the DeviceMemoryProperties which support these combinations of memory properties");
      return static_cast<uint32_t>(-1);
   };

   VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
   uint64_t allocatedSize = 0u;
   {
      // Allocate the memory
      VkMemoryAllocateInfo memoryAllocateInfo = {};
      memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memoryAllocateInfo.pNext = nullptr;
      memoryAllocateInfo.allocationSize = p_memoryRequirements.size;
      memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(p_memoryRequirements.memoryTypeBits, p_memoryProperties);
      [[maybe_unused]] const VkResult res = vkAllocateMemory(GetLogicalDeviceNative(), &memoryAllocateInfo, nullptr, &deviceMemory);

      ASSERT(res == VK_SUCCESS, "Failed to allocate the device memory for the buffer");

      allocatedSize = p_memoryRequirements.size;
   }

   return {deviceMemory, allocatedSize};
}

void Device::QueueSubmitInternal(QueueFamilyType p_executingQueueType, std::vector<Ptr<GHI::CommandBuffer>> p_commandBuffers,
                                 std::vector<FenceSubmitInfo> p_waitFence, std::vector<FenceSubmitInfo> p_signalAfter)
{
   std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
   waitSemaphores.reserve(p_waitFence.size());
   {
      for (const FenceSubmitInfo& fence : p_waitFence)
      {
         // NOTE: VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT is not ideal, but DX12 doesn't support it, so we'll have to support the least
         // common denominator
         waitSemaphores.emplace_back(VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr,
                                     Cast<Vulkan::Fence>(fence.m_fence)->GetTimelineSemaphoreNative(), fence.m_value,
                                     VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0u);
      }
   }

   std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
   signalSemaphores.reserve(p_signalAfter.size());
   {
      for (const FenceSubmitInfo& fence : p_signalAfter)
      {
         // NOTE: VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT is not ideal, but DX12 doesn't support it, so we'll have to support the least
         // common denominator
         signalSemaphores.emplace_back(VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr,
                                       Cast<Vulkan::Fence>(fence.m_fence)->GetTimelineSemaphoreNative(), fence.m_value,
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
