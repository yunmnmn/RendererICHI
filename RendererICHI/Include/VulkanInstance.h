#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Std/vector.h>
#include <Std/unordered_map.h>

#include <Memory/AllocatorClass.h>
#include <Util/HashName.h>

#include <RenderResource.h>
#include <RenderWindow.h>
#include <VulkanDevice.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class RenderState;

struct VulkanInstanceDescriptor
{
   Foundation::Util::HashName m_instanceName;
   uint32_t m_version = VK_API_VERSION_1_2;
   bool m_debug = false;
   Std::vector<const char*> m_layers;
   Std::vector<const char*> m_instanceExtensions;
};

class VulkanInstance final : public RenderResource<VulkanInstance>
{
   static constexpr uint32_t InvalidPhysicalDeviceIndex = static_cast<uint32_t>(-1);
   static constexpr uint32_t InvalidQueueFamilyIndex = InvalidPhysicalDeviceIndex;

 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VulkanInstance, 1u);

   VulkanInstance() = delete;
   VulkanInstance(VulkanInstanceDescriptor&& p_desc);
   ~VulkanInstance() final;

   // Get PhysicalDevices
   const Std::vector<VkPhysicalDevice> GetPhysicalDevicesNative() const;
   const VkPhysicalDevice GetPhysicalDeviceNative(uint32_t p_physicalDeviceIndex) const;

   // Get PhysicalDevice count
   uint32_t GetPhysicalDevicesCount() const;

   // VulkanInstanceInterface overrides...
   VkInstance GetInstanceNative() const;
   bool IsLayerUsed(Foundation::Util::HashName layerName) const;
   bool IsExtensionUsed(Foundation::Util::HashName extensionName) const;

 private:
   void EnableDebugging();

   Std::vector<VkPhysicalDevice> m_physicalDevices;

   VkApplicationInfo m_applicationInfo;
   Std::vector<Foundation::Util::HashName> m_instanceLayers;
   Std::vector<Foundation::Util::HashName> m_instanceExtensions;
   Std::vector<VkLayerProperties> m_instanceLayerProperties;
   Std::vector<VkExtensionProperties> m_instanceExtensionProperties;
   VkInstance m_instance = VK_NULL_HANDLE;

   uint32_t m_physicalDeviceIndex = InvalidPhysicalDeviceIndex;

   VkDebugUtilsMessengerEXT m_debugUtilsMessenger;

   bool m_debugging = false;
};
}; // namespace Render
