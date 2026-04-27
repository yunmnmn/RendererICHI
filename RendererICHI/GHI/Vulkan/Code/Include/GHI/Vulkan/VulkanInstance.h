#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <unordered_map>
#include <string>
#include <span>

#include <vulkan/vulkan.h>

#include <GHI/Device.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

struct VulkanInstanceDescriptor
{
   std::string m_instanceName;
   uint32_t m_version = VK_API_VERSION_1_3;
   bool m_debug = false;
   std::vector<const char*> m_layers;
   std::vector<const char*> m_instanceExtensions;
};

class VulkanInstance
{
 public:
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

   bool IsLayerUsed(std::string_view layerName) const;

   bool IsExtensionUsed(std::string_view extensionName) const;

   const bool IsDebugEnabled() const
   {
      return m_debugging;
   }

   std::vector<Ptr<GHI::PhysicalDevice>> GetPhysicalDevices();

 private:
   void CreatePhysicalDevices(VkInstance p_instance);

   void EnableDebugging();

   std::vector<Ptr<GHI::PhysicalDevice>> m_physicalDevices;

   VkApplicationInfo m_applicationInfo;
   std::vector<std::string> m_instanceLayers;
   std::vector<std::string> m_instanceExtensions;
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
