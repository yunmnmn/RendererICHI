#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/unique_ptr.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <Memory/ClassAllocator.h>
#include <Util/HashName.h>

#include <Device.h>

#include <glad/vulkan.h>

namespace Render
{
class RenderInstance
{
 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(RenderInstance, 1u, static_cast<uint32_t>(sizeof(RenderInstance)));

   struct Descriptor
   {
      Foundation::Util::HashName m_instanceName;
      uint32_t m_version = 0;
   };
   static eastl::unique_ptr<RenderInstance> CreateInstance(Descriptor&& p_desc);

   RenderInstance(Descriptor p_desc);
   ~RenderInstance();

   void AddLayer(Render::vector<Foundation::Util::HashName>&& layers);
   void AddExtension(Render::vector<Foundation::Util::HashName>&& extensions);

   void CompileInstance();

   const VkInstance& GetInstance() const;

 private:
   RenderInstance() = delete;

   VkApplicationInfo m_applicationInfo;
   Render::vector<Device> m_devices;

   Render::vector<Foundation::Util::HashName> m_instanceLayers;
   Render::vector<Foundation::Util::HashName> m_instanceExtensions;

   Render::vector<VkLayerProperties> m_instanceLayerProperties;
   Render::vector<VkExtensionProperties> m_instanceExtensionProperties;

   VkInstance m_instance;
};
}; // namespace Render
