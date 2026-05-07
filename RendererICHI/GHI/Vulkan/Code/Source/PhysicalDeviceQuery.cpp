#include <GHI/Vulkan/PhysicalDeviceQuery.h>

#include <span>
#include <algorithm>
#include <cstring>

#include <GLFW/glfw3.h>

#include <Util/MurmurHash3.h>
#include <Util/Assert.h>

#include <GHI/Vulkan/PhysicalDevice.h>
#include <GHI/Vulkan/VulkanInstance.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- QueueFamilyHandle -----------
QueueFamilyHandle::QueueFamilyHandle(uint32_t p_queueFamilyIndex, uint32_t p_queueIndex)
{
   m_queueFamilyIndex = p_queueFamilyIndex;
   m_queueIndex = p_queueIndex;
}

bool QueueFamilyHandle::IsValid() const
{
   return m_queueFamilyIndex != InvalidQueueFamilyIndex && m_queueIndex != InvalidQueueFamilyIndex;
}

uint32_t QueueFamilyHandle::GetQueueFamilyIndex() const
{
   return m_queueFamilyIndex;
}

uint32_t QueueFamilyHandle::GetQueueIndex() const
{
   return m_queueIndex;
}

uint64_t QueueFamilyHandle::CalculateHash() const
{
   return MurmurHash3_x64_64_Helper<QueueFamilyHandle>(this);
}

bool QueueFamilyHandle::operator==(const QueueFamilyHandle& other) const
{
   return this->CalculateHash() == other.CalculateHash();
}

size_t QueueFamilyHandle::operator()(const QueueFamilyHandle& p_handle) const
{
   return p_handle.CalculateHash();
}

// ----------- QueueFamily -----------

QueueFamily::QueueFamily(VkQueueFamilyProperties p_queueFamilyProperties, uint32_t p_queueFamilyIndex)
{
   m_queueFamilyProperties = p_queueFamilyProperties;
   m_queueFamilyIndex = p_queueFamilyIndex;
}

bool QueueFamily::SupportFlags(VkQueueFlags queueFlags) const
{
   return (m_queueFamilyProperties.queueFlags & queueFlags) == queueFlags;
}

uint32_t QueueFamily::GetQueueCount() const
{
   return m_queueFamilyProperties.queueCount;
}

uint32_t QueueFamily::GetAllocatedQueueCount() const
{
   return m_allocatedQueueCount;
}

QueueFamilyHandle QueueFamily::CreateQueueFamilyHandle()
{
   QueueFamilyHandle queueFamilyHandle(m_queueFamilyIndex, m_allocatedQueueCount);

   m_allocatedQueueCount++;

   return queueFamilyHandle;
}

bool QueueFamily::AvailableQueue() const
{
   return (m_allocatedQueueCount < GetQueueCount());
}

uint32_t QueueFamily::GetSupportedQueuesCount() const
{
   // TODO: this might fail if the queues get updated on Vulkan's side
   const uint32_t QueueTypeCount = 5u;
   uint32_t supportedQueueTypes = 0u;
   for (uint32_t i = 0; i < QueueTypeCount; i++)
   {
      if (m_queueFamilyProperties.queueFlags & (1 << i))
      {
         supportedQueueTypes++;
      }
   }

   return supportedQueueTypes;
}

QueueTypeFlags QueueFamily::GetSupportedQueueTypeFlags() const
{
   QueueTypeFlags queueTypes = QueueTypeFlags::None;
   if ((m_queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u)
   {
      queueTypes |= QueueTypeFlags::GraphicsQueue;
   }
   if ((m_queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u)
   {
      queueTypes |= QueueTypeFlags::ComputeQueue;
   }
   if ((m_queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u)
   {
      queueTypes |= QueueTypeFlags::TransferQueue;
   }
   return queueTypes;
}

// ----------- SurfaceQuery -----------

SurfaceQuery::SurfaceQuery(VkPhysicalDevice p_device, VkSurfaceKHR p_surface)
{
   // Get the device surface capabilities
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_device, p_surface, &m_capabilities);

   // Get the device's surface formats
   {
      uint32_t formatCount = 0u;
      vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, p_surface, &formatCount, nullptr);
      m_formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, p_surface, &formatCount, m_formats.data());
   }

   // Get the device's present modes
   {
      uint32_t presentModeCount = 0;
      vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, p_surface, &presentModeCount, nullptr);

      m_presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, p_surface, &presentModeCount, m_presentModes.data());
   }
}

const VkSurfaceCapabilitiesKHR& SurfaceQuery::GetSurfaceCapabilities() const
{
   return m_capabilities;
}

std::span<const VkSurfaceFormatKHR> SurfaceQuery::GetSupportedFormats() const
{
   return std::span<const VkSurfaceFormatKHR>(m_formats);
}

std::span<const VkPresentModeKHR> SurfaceQuery::GetSupportedPresentModes() const
{
   return std::span<const VkPresentModeKHR>(m_presentModes);
}

bool SurfaceQuery::SupportSwapchain() const
{
   return (GetSupportedFormats().size() != 0u && GetSupportedPresentModes().size() != 0u);
}

// ----------- PhysicalDeviceQuery -----------

PhysicalDeviceQuery::PhysicalDeviceQuery(VkInstance p_instance, VkPhysicalDevice p_physicalDevice)
{
   m_instance = p_instance;
   m_physicalDevice = p_physicalDevice;

   // Get the physical device specific properties
   {
      m_descriptorBufferProperties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT};

      VkPhysicalDeviceProperties2 physicalDeviceProperties = {};
      physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
      physicalDeviceProperties.pNext = &m_descriptorBufferProperties;

      vkGetPhysicalDeviceProperties2(m_physicalDevice, &physicalDeviceProperties);
      m_physicalDeviceProperties = physicalDeviceProperties.properties;
   }

   // Get the supported physical device memory properties
   vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);

   // Get list of supported extensions
   uint32_t extensionCount = 0u;
   vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
   m_extensionProperties.resize(extensionCount);
   vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, m_extensionProperties.data());

   // Create the queue family properties
   {
      // Find the supported PhysicalDevice's family queue's
      uint32_t queueFamilyCount = 0u;
      vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
      ASSERT(queueFamilyCount > 0u, "No supported physical devices on this machine");

      // Get the queue family properties
      std::vector<VkQueueFamilyProperties> queueFamilyProperties;
      queueFamilyProperties.resize(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

      // Create the QueueFamily instances
      m_queueFamilyArray.reserve(queueFamilyProperties.size());
      for (uint32_t i = 0u; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
      {
         m_queueFamilyArray.emplace_back(queueFamilyProperties[i], i);
      }
   }

   // Find all the queues
   {
      m_graphicsQueueFamilyHandle =
          GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_graphicsQueueFamilyHandle.GetQueueFamilyIndex() != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      m_computeQueueFamilyHandle = GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_computeQueueFamilyHandle.GetQueueFamilyIndex() != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      m_transferQueueFamilyHandle = GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_transferQueueFamilyHandle.GetQueueFamilyIndex() != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      // Find the most suited presenting QueueFamily index
      m_presentQueueFamilyHandle = GetSuitedPresentQueueFamilyIndex();
   }

   QueryPhysicalDeviceFeatures();
   QuerySupportedQueues();
   QuerySupportedFeatures();
   QueryGpuType();
}

void PhysicalDeviceQuery::QueryPhysicalDeviceFeatures()
{
   vertexInputDynamicState = {};
   vertexInputDynamicState.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
   vertexInputDynamicState.pNext = nullptr;

   synchronization2Features = {};
   synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
   synchronization2Features.pNext = &vertexInputDynamicState;

   colorWriteCreateInfo = {};
   colorWriteCreateInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT;
   colorWriteCreateInfo.pNext = &synchronization2Features;

   dynamicState = {};
   dynamicState.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
   dynamicState.pNext = &colorWriteCreateInfo;

   dynamicState2 = {};
   dynamicState2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
   dynamicState2.pNext = &dynamicState;

   dynamicRenderingFeatures = {};
   dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
   dynamicRenderingFeatures.pNext = &dynamicState2;

   supportedVulkan12Features = {};
   supportedVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
   supportedVulkan12Features.pNext = &dynamicRenderingFeatures;

   supportedVulkan13Features = {};
   supportedVulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
   supportedVulkan13Features.pNext = &supportedVulkan12Features;

   mutableDescriptorType = {};
   mutableDescriptorType.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT;
   mutableDescriptorType.pNext = &supportedVulkan13Features;

   descriptorBufferFeatures = {};
   descriptorBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
   descriptorBufferFeatures.pNext = &mutableDescriptorType;

   void* featureChain = &descriptorBufferFeatures;
   meshShaderFeatures = {};
   if (IsDeviceExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME))
   {
      meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
      meshShaderFeatures.pNext = featureChain;
      featureChain = &meshShaderFeatures;
   }

   VkPhysicalDeviceFeatures2 deviceFeatures = {};
   deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
   deviceFeatures.pNext = featureChain;

   vkGetPhysicalDeviceFeatures2(m_physicalDevice, &deviceFeatures);
}

void PhysicalDeviceQuery::QuerySupportedQueues()
{
   if (m_graphicsQueueFamilyHandle.IsValid())
   {
      m_supportedQueues |= QueueTypeFlags::GraphicsQueue;
   }
   if (m_computeQueueFamilyHandle.IsValid())
   {
      m_supportedQueues |= QueueTypeFlags::ComputeQueue;
   }
   if (m_transferQueueFamilyHandle.IsValid())
   {
      m_supportedQueues |= QueueTypeFlags::TransferQueue;
   }
}

void PhysicalDeviceQuery::QuerySupportedFeatures()
{
   if (IsDeviceExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
   {
      m_supportedFeatures |= PhysicalDeviceFeatureFlags::Swapchain;
   }

   if (SupportPresenting())
   {
      m_supportedFeatures |= PhysicalDeviceFeatureFlags::Presenting;
   }

   if (IsDeviceExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME) && meshShaderFeatures.meshShader)
   {
      m_supportedFeatures |= PhysicalDeviceFeatureFlags::MeshShader;
   }

   if (IsDeviceExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) && dynamicState.extendedDynamicState)
   {
      m_supportedFeatures |= PhysicalDeviceFeatureFlags::ExtendedDynamicState;
   }

   if (IsDeviceExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME) && dynamicState2.extendedDynamicState2)
   {
      m_supportedFeatures |= PhysicalDeviceFeatureFlags::ExtendedDynamicState2;
   }
}

void PhysicalDeviceQuery::QueryGpuType()
{
   if (IsDiscreteGpu())
   {
      m_type = GPUType::Discrete;
   }
   else if (IsIntegratedGpu())
   {
      m_type = GPUType::Integrated;
   }
}

QueueFamilyHandle PhysicalDeviceQuery::GetSuitedQueueFamilyHandle(VkQueueFlagBits queueFlags)
{
   // Heuristic:
   // Check if the QueueFamily supports all flags
   // Use the QueueFamily Index that support the least amount of Queues
   uint32_t queueFamilyIndex = InvalidQueueFamilyIndex;
   uint32_t queueFamilyQueueCount = static_cast<uint32_t>(-1);
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_queueFamilyArray.size()); i++)
   {
      if (m_queueFamilyArray[i].SupportFlags(queueFlags) && m_queueFamilyArray[i].AvailableQueue())
      {
         const uint32_t currentQueueCount = m_queueFamilyArray[i].GetSupportedQueuesCount();
         if (currentQueueCount < queueFamilyQueueCount)
         {
            queueFamilyIndex = i;
            queueFamilyQueueCount = currentQueueCount;
         }
      }
   }

   // TODO: If it's still invalid, occupy a FamilyQueue

   if (queueFamilyIndex != InvalidQueueFamilyIndex)
   {
      // Create the handle
      return m_queueFamilyArray[queueFamilyIndex].CreateQueueFamilyHandle();
   }
   else
   {
      return QueueFamilyHandle();
   }
}

uint32_t PhysicalDeviceQuery::GetSuitedPresentQueueFamilyIndex() const
{
   // Check if the graphics queue is supporting presentation
   if (glfwGetPhysicalDevicePresentationSupport(m_instance, m_physicalDevice, m_graphicsQueueFamilyHandle.GetQueueFamilyIndex()))
   {
      return m_graphicsQueueFamilyHandle.GetQueueFamilyIndex();
   }
   else
   {
      // Else get the first index in the list
      return GetPresentingFamilyQueueIndex();
   }
}

bool PhysicalDeviceQuery::IsDeviceExtensionSupported(std::string_view p_deviceExtension) const
{
   const auto extenstionItr = std::find_if(m_extensionProperties.begin(), m_extensionProperties.end(),
                                           [p_deviceExtension](const VkExtensionProperties& extension) {
                                              return strcmp(extension.extensionName, p_deviceExtension.data()) == 0;
                                           });

   return extenstionItr != m_extensionProperties.end();
}

uint32_t PhysicalDeviceQuery::SupportQueueFamilyFlags(VkQueueFlags queueFlags) const
{
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_queueFamilyArray.size()); i++)
   {
      if (m_queueFamilyArray[i].SupportFlags(queueFlags))
      {
         return i;
      }
   }

   return InvalidQueueFamilyIndex;
}

uint32_t PhysicalDeviceQuery::GetPresentingFamilyQueueIndex() const
{
   // Check if presenting is supported in the physical device
   for (uint32_t j = 0; j < GetQueueFamilyCount(); j++)
   {
      if (glfwGetPhysicalDevicePresentationSupport(m_instance, m_physicalDevice, j))
      {
         return j;
      }
   }

   return InvalidQueueFamilyIndex;
}

bool PhysicalDeviceQuery::IsDiscreteGpu() const
{
   return m_physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool PhysicalDeviceQuery::IsIntegratedGpu() const
{
   return m_physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

bool PhysicalDeviceQuery::IsViable() const
{
   ASSERT(m_physicalDevice != VK_NULL_HANDLE, "Invalid native physical device");

   // TODO: add more
   return
       // Dynamic Rendering support
       supportedVulkan13Features.dynamicRendering && dynamicRenderingFeatures.dynamicRendering &&
       // Timeline Semaphore support
       supportedVulkan12Features.timelineSemaphore &&
       // Device address support, required by descriptor buffers
       supportedVulkan12Features.bufferDeviceAddress &&
       // Synchronizing2 support
       supportedVulkan13Features.synchronization2 && synchronization2Features.synchronization2 &&
       // Shader indexing support
       supportedVulkan12Features.shaderInputAttachmentArrayDynamicIndexing &&
       supportedVulkan12Features.shaderUniformTexelBufferArrayDynamicIndexing &&
       supportedVulkan12Features.shaderUniformBufferArrayNonUniformIndexing &&
       // Mutable Descriptor support
       mutableDescriptorType.mutableDescriptorType &&
       // Descriptor Buffer related features
       descriptorBufferFeatures.descriptorBuffer && descriptorBufferFeatures.descriptorBufferPushDescriptors;
}

uint32_t PhysicalDeviceQuery::GetPresentableFamilyQueueIndex() const
{
   ASSERT(m_graphicsQueueFamilyHandle.GetQueueFamilyIndex() != InvalidQueueFamilyIndex,
          "Presentable family queue index is invalid");
   return m_graphicsQueueFamilyHandle.GetQueueFamilyIndex();
}

uint32_t PhysicalDeviceQuery::GetQueueFamilyCount() const
{
   return static_cast<uint32_t>(m_queueFamilyArray.size());
}

QueueFamilyHandle PhysicalDeviceQuery::GetGraphicsQueueFamilyHandle() const
{
   return m_graphicsQueueFamilyHandle;
}

QueueFamilyHandle PhysicalDeviceQuery::GetComputeQueueFamilyHandle() const
{
   return m_computeQueueFamilyHandle;
}

QueueFamilyHandle PhysicalDeviceQuery::GetTransferQueueFamilyHandle() const
{
   return m_transferQueueFamilyHandle;
}

QueueFamilyInfo PhysicalDeviceQuery::GetQueueFamilyInfo(QueueFamilyType p_queueType) const
{
   QueueFamilyHandle handle;
   switch (p_queueType)
   {
   case QueueFamilyType::GraphicsQueue:
      handle = m_graphicsQueueFamilyHandle;
      break;
   case QueueFamilyType::ComputeQueue:
      handle = m_computeQueueFamilyHandle;
      break;
   case QueueFamilyType::TransferQueue:
      handle = m_transferQueueFamilyHandle;
      break;
   default:
      return QueueFamilyInfo{};
   }

   ASSERT(handle.IsValid(), "Requested queue type does not have a selected queue family");
   ASSERT(handle.GetQueueFamilyIndex() < m_queueFamilyArray.size(), "Selected queue family is out of range");

   return QueueFamilyInfo{.m_queueType = p_queueType,
                          .m_supportedQueues =
                              m_queueFamilyArray[handle.GetQueueFamilyIndex()].GetSupportedQueueTypeFlags(),
                          .m_familyIndex = handle.GetQueueFamilyIndex(),
                          .m_queueIndex = handle.GetQueueIndex()};
}

const VkPhysicalDeviceMemoryProperties& PhysicalDeviceQuery::GetMemoryProperties() const
{
   return m_deviceMemoryProperties;
}

const VkPhysicalDeviceDescriptorBufferPropertiesEXT& PhysicalDeviceQuery::GetDescriptorBufferPropertiesEXT() const
{
   return m_descriptorBufferProperties;
}

QueueTypeFlags PhysicalDeviceQuery::GetQueueTypeFlags() const
{
   return m_supportedQueues;
}

PhysicalDeviceFeatureFlags PhysicalDeviceQuery::GetPhysicalDeviceFeatureFlags() const
{
   return m_supportedFeatures;
}

GPUType PhysicalDeviceQuery::GetGPUTypes() const
{
   return m_type;
}

bool PhysicalDeviceQuery::SupportPresenting() const
{
   return m_presentQueueFamilyHandle != InvalidQueueFamilyIndex;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
