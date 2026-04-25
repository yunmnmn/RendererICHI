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
   static constexpr uint32_t InvalidQueueFamilyIndex = static_cast<uint32_t>(-1);

 public:
   PhysicalDevice() = delete;
   PhysicalDevice(VkPhysicalDevice p_physicalDeviceNative, PhysicalDeviceDescriptor&& p_desc);

   void ReleaseInternal() final;

 public:
   ~PhysicalDevice() final;

 public:
   std::tuple<VkDeviceMemory, uint64_t> AllocateDeviceMemory(VkMemoryRequirements p_memoryRequirements,
                                                             MemoryPropertyFlags p_memoryProperties);

   VkPhysicalDevice GetPhysicalDeviceNative() const;

 public:
   ///////////////////////////////////////////////////
   // GHI::PhysicalDevice
   QueueTypeFlags GetQueueTypeFlags() const final;
   PhysicalDeviceFeatureFlags GetPhysicalDeviceFeatureFlags() const final;
   GPUType GetGPUTypes() const final;
   bool IsViable() const final;
   ///////////////////////////////////////////////////

 private:
   PhysicalDeviceQuery m_physicalDeviceQuery;
   VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

   QueueTypeFlags m_supportedQueues = {};
   PhysicalDeviceFeatureFlags m_supportedFeatures = {};
   GPUType m_type = {};
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
