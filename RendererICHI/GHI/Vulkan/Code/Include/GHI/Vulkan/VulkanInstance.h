#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <Memory/AllocatorClass.h>
#include <Util/HashName.h>

#include <GHI/Device.h>

using namespace Foundation;

namespace Render
{
namespace GHI
{

namespace Vulkan
{

struct VulkanInstanceDescriptor
{
   Foundation::Util::HashName m_instanceName;
   uint32_t m_version = VK_API_VERSION_1_3;
   bool m_debug = false;
   std::vector<const char*> m_layers;
   std::vector<const char*> m_instanceExtensions;
};

class VulkanInstance
{
 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VulkanInstance, 1u);

   static constexpr uint32_t InvalidPhysicalDeviceIndex = static_cast<uint32_t>(-1);
   static constexpr uint32_t InvalidQueueFamilyIndex = InvalidPhysicalDeviceIndex;

 public:
   static VulkanInstance* Get();

   VulkanInstance();
   ~VulkanInstance();

   void Init(VulkanInstanceDescriptor&& p_desc);
   void Shutdown();

   // Get PhysicalDevice count
   uint32_t GetPhysicalDevicesCount() const;

   // VulkanInstanceInterface overrides...
   VkInstance GetInstanceNative() const;

   bool IsLayerUsed(Foundation::Util::HashName layerName) const;

   bool IsExtensionUsed(Foundation::Util::HashName extensionName) const;

   const bool IsDebugEnabled() const
   {
      return m_debugging;
   }

   std::span<const Ptr<PhysicalDevice>> GetPhysicalDevices() const;
   std::span<Ptr<PhysicalDevice>> GetPhysicalDevices();

 private:
   void CreatePhysicalDevices();

   void EnableDebugging();

   std::vector<Ptr<PhysicalDevice>> m_physicalDevices;

   VkApplicationInfo m_applicationInfo;
   std::vector<Foundation::Util::HashName> m_instanceLayers;
   std::vector<Foundation::Util::HashName> m_instanceExtensions;
   std::vector<VkLayerProperties> m_instanceLayerProperties;
   std::vector<VkExtensionProperties> m_instanceExtensionProperties;
   VkInstance m_instance = VK_NULL_HANDLE;

   uint32_t m_physicalDeviceIndex = InvalidPhysicalDeviceIndex;

   VkDebugUtilsMessengerEXT m_debugUtilsMessenger;

   bool m_debugging = false;
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
