#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/unique_ptr.h>
#include <std/vector.h>

#include <Memory/ClassAllocator.h>
#include <Util/HashName.h>

#include <glad/vulkan.h>

namespace Render
{
   // TODO: move the definition to the cpp
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

      static eastl::unique_ptr<RenderInstance> CreateInstance(Descriptor&& p_desc)
      {
         return eastl::make_unique<RenderInstance>(p_desc);
      }

      ~RenderInstance()
      {

      }

      void AddExtension(Render::vector<Foundation::Util::HashName>&& extensions)
      {
      }

      void CompileInstance()
      {
      }

   private:
      RenderInstance() = default;
      RenderInstance(Descriptor&& p_desc)
      {
         m_applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
         m_applicationInfo.pApplicationName = p_desc.m_instanceName.GetCStr();
         m_applicationInfo.pEngineName = p_desc.m_instanceName.GetCStr();
         m_applicationInfo.apiVersion = p_desc.m_version;
      }

      VkApplicationInfo m_applicationInfo;
      Render::vector<Foundation::Util::HashName> m_instanceExtensions;
      Render::vector<class Device> m_devices;
   };
}; // namespace Render
