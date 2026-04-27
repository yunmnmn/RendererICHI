#include <GHI/Vulkan/PhysicalDevice.h>

#include <GLFW/glfw3.h>

#include <Util/Macro.h>
#include <Util/Assert.h>
#include <Util/MurmurHash3.h>

#include <GHI/Vulkan/VulkanInstance.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- PhysicalDevice -----------

PhysicalDevice::PhysicalDevice(VkPhysicalDevice p_physicalDeviceNative, PhysicalDeviceDescriptor&& p_desc)
    : GHI::PhysicalDevice(std::move(p_desc)), m_physicalDeviceQuery(p_physicalDeviceNative)
{
   m_physicalDevice = p_physicalDeviceNative;

   // Create a temporary window
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
   GLFWwindow* windowNative = glfwCreateWindow(0u, 0u, nullptr, nullptr, nullptr);
   ASSERT(windowNative, "Failed to create a window");

   // Create the temporary surface
   VkSurfaceKHR surface = VK_NULL_HANDLE;
   const VkResult result =
       glfwCreateWindowSurface(Vulkan::VulkanInstance::Get()->GetInstanceNative(), windowNative, nullptr, &surface);
   ASSERT(result == VK_SUCCESS, "Failed to create the window surface");

   {
      PhysicalDeviceQuery physicalDeviceQuery(m_physicalDevice);
      SurfaceQuery surfaceQuery(m_physicalDevice, surface);

      if (physicalDeviceQuery.GetGraphicsQueueFamilyHandle().IsValid())
      {
         m_supportedQueues |= QueueTypeFlags::GraphicsQueue;
      }
      if (physicalDeviceQuery.GetComputeQueueFamilyHandle().IsValid())
      {
         m_supportedQueues |= QueueTypeFlags::ComputeQueue;
      }
      if (physicalDeviceQuery.GetTransferQueueFamilyHandle().IsValid())
      {
         m_supportedQueues |= QueueTypeFlags::TransferQueue;
      }

      if (physicalDeviceQuery.SupportPresenting())
      {
         m_supportedFeatures |= PhysicalDeviceFeatureFlags::Presenting;
      }
      if (surfaceQuery.SupportSwapchain())
      {
         m_supportedFeatures |= PhysicalDeviceFeatureFlags::Swapchain;
      }

      if (physicalDeviceQuery.IsDiscreteGpu())
      {
         m_type |= GPUType::Discrete;
      }
      else if (physicalDeviceQuery.IsIntegratedGpu())
      {
         m_type |= GPUType::Integrated;
      }
   }

   // Destroy surface and window again
   vkDestroySurfaceKHR(Vulkan::VulkanInstance::Get()->GetInstanceNative(), surface, nullptr);
   glfwDestroyWindow(windowNative);
}

PhysicalDevice::~PhysicalDevice()
{
}

void PhysicalDevice::ReleaseInternal()
{
}

VkPhysicalDevice PhysicalDevice::GetPhysicalDeviceNative() const
{
   return m_physicalDevice;
}

bool PhysicalDevice::IsDeviceExtensionSupported(std::string_view p_deviceExtension) const
{
   return m_physicalDeviceQuery.IsDeviceExtensionSupported(p_deviceExtension);
}

QueueFamilyHandle PhysicalDevice::GetGraphicsQueueFamilyHandle() const
{
   return m_physicalDeviceQuery.GetGraphicsQueueFamilyHandle();
}

QueueFamilyHandle PhysicalDevice::GetComputeQueueFamilyHandle() const
{
   return m_physicalDeviceQuery.GetComputeQueueFamilyHandle();
}

QueueFamilyHandle PhysicalDevice::GetTransferQueueFamilyHandle() const
{
   return m_physicalDeviceQuery.GetTransferQueueFamilyHandle();
}

QueueTypeFlags PhysicalDevice::GetQueueTypeFlags() const
{
   return m_supportedQueues;
}

PhysicalDeviceFeatureFlags PhysicalDevice::GetPhysicalDeviceFeatureFlags() const
{
   return m_supportedFeatures;
}

GPUType PhysicalDevice::GetGPUTypes() const
{
   return m_type;
}

bool PhysicalDevice::IsViable() const
{
   ASSERT(m_physicalDevice != VK_NULL_HANDLE, "Invalid native physical device");

   // Get the physical device specific properties
   VkPhysicalDeviceProperties physicalDeviceProperties = {};
   vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

   // Query support for extensions
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
   VkPhysicalDeviceFeatures2 deviceFeatures = {};
   {
      vertexInputDynamicState.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
      vertexInputDynamicState.pNext = nullptr;

      synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
      synchronization2Features.pNext = &vertexInputDynamicState;

      colorWriteCreateInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT;
      colorWriteCreateInfo.pNext = &synchronization2Features;

      // Check for extended DynamicState support
      dynamicState.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
      dynamicState.pNext = &colorWriteCreateInfo;

      // Check for extended DynamicState2 support
      dynamicState2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
      dynamicState2.pNext = &dynamicState;

      // Check for Dynamic Rendering support
      dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
      dynamicRenderingFeatures.pNext = &dynamicState2;

      // Check for Vulkan 1.2 support
      supportedVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
      supportedVulkan12Features.pNext = &dynamicRenderingFeatures;

      // Check for Vulkan 1.3 support
      supportedVulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
      supportedVulkan13Features.pNext = &supportedVulkan12Features;

      // Check for Mutable Descriptor support
      mutableDescriptorType.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT;
      mutableDescriptorType.pNext = &supportedVulkan13Features;

      // Check for Descriptor Buffer support
      descriptorBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
      descriptorBufferFeatures.pNext = &mutableDescriptorType;

      deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
      deviceFeatures.pNext = static_cast<void*>(&descriptorBufferFeatures);
      vkGetPhysicalDeviceFeatures2(m_physicalDevice, &deviceFeatures);

      // TODO: add more
      if (
          // Dynamic State support
          dynamicState.extendedDynamicState && dynamicState2.extendedDynamicState2 &&
          // Dynamic Rendering support
          supportedVulkan13Features.dynamicRendering && dynamicRenderingFeatures.dynamicRendering &&
          // Timeline Semaphore support
          supportedVulkan12Features.timelineSemaphore &&
          // Synchronizing2 support
          supportedVulkan13Features.synchronization2 && synchronization2Features.synchronization2 &&
          // Shader indexing support
          supportedVulkan12Features.shaderInputAttachmentArrayDynamicIndexing &&
          supportedVulkan12Features.shaderUniformTexelBufferArrayDynamicIndexing &&
          supportedVulkan12Features.shaderUniformBufferArrayNonUniformIndexing &&
          // Mutable Descriptor support
          mutableDescriptorType.mutableDescriptorType &&
          // Descriptor Buffer related features
          descriptorBufferFeatures.descriptorBuffer && descriptorBufferFeatures.descriptorBufferPushDescriptors &&
          // etc.
          vertexInputDynamicState.vertexInputDynamicState && colorWriteCreateInfo.colorWriteEnable)
      {
         return true;
      }
   }

   return false;
}

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
