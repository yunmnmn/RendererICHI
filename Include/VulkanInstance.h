#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/unique_ptr.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <Memory/ClassAllocator.h>
#include <Util/HashName.h>
#include <ResourceReference.h>

#include <VulkanDevice.h>

#include <glad/vulkan.h>

#include <VulkanInstanceInterface.h>

namespace Render
{
class VulkanDevice;

class VulkanInstance : public VulkanInstanceInterface, : public RenderResource<VulkanInstance, VulkanInstance::Descriptor>
{
   static constexpr uint32_t InvalidPhysicalDeviceIndex = static_cast<uint32_t>(-1);

 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VulkanInstance, 1u, static_cast<uint32_t>(sizeof(VulkanInstance)));

   struct Descriptor
   {
      Foundation::Util::HashName m_instanceName;
      uint32_t m_version = VK_API_VERSION_1_2;
      Render::vector<const char*> m_layers;
      Render::vector<const char*> m_extensions;
   };

   VulkanInstance() = delete;
   VulkanInstance(Descriptor p_desc);
   ~VulkanInstance();

   void EnableDebugging();

   // Create the physical devices
   void CreatePhysicalDevices();

   // NOTE: only support a single device right now
   // Create the logical device on the physical device that supports all extensions
   void SelectAndCreateLogicalDevice(Render::vector<const char*>&& p_deviceExtensions);

   // VulkanInstanceInterface overrides...
   const VkInstance& GetInstance() const final;
   bool IsLayerUsed(Foundation::Util::HashName layerName) const final;
   bool IsExtensionUsed(Foundation::Util::HashName extensionName) const final;
   VulkanDevice* GetSelectedPhysicalDevice() final;

 private:
   VkApplicationInfo m_applicationInfo;
   Render::vector<Foundation::Util::HashName> m_instanceLayers;
   Render::vector<Foundation::Util::HashName> m_instanceExtensions;
   Render::vector<VkLayerProperties> m_instanceLayerProperties;
   Render::vector<VkExtensionProperties> m_instanceExtensionProperties;
   Render::vector<eastl::unique_ptr<VulkanDevice>> m_physicalDevices;
   VkInstance m_instance = VK_NULL_HANDLE;

   uint32_t m_physicalDeviceIndex = InvalidPhysicalDeviceIndex;

   VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
};
}; // namespace Render
