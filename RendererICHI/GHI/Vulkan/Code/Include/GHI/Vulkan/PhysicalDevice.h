#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <cstdint>

#include <vulkan/vulkan.h>

#include <GHI/PhysicalDevice.h>

#include <GHI/Vulkan/PhysicalDeviceQuery.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- PhysicalDevice -----------

class PhysicalDevice final : public GHI::PhysicalDevice
{
 public:
   PhysicalDevice() = delete;
   PhysicalDevice(VkInstance p_instance, VkPhysicalDevice p_physicalDeviceNative, PhysicalDeviceDescriptor&& p_desc);

   void ReleaseInternal() final;

 public:
   ~PhysicalDevice() final;

 public:
   VkPhysicalDevice GetPhysicalDeviceNative() const;
   bool IsDeviceExtensionSupported(std::string_view p_deviceExtension) const;

   QueueFamilyHandle GetGraphicsQueueFamilyHandle() const;
   QueueFamilyHandle GetComputeQueueFamilyHandle() const;
   QueueFamilyHandle GetTransferQueueFamilyHandle() const;
   const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const;
   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& GetDescriptorBufferPropertiesEXT() const;

 public:
   ///////////////////////////////////////////////////
   // GHI::PhysicalDevice
   QueueTypeFlags GetQueueTypeFlags() const final;
   PhysicalDeviceFeatureFlags GetPhysicalDeviceFeatureFlags() const final;
   GPUType GetGPUTypes() const final;
   QueueFamilyInfo GetQueueFamilyInfo(QueueFamilyType p_queueType) const final;
   bool IsViable() const final;
   ///////////////////////////////////////////////////

 private:
   PhysicalDeviceQuery m_physicalDeviceQuery;
   VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
