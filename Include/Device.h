#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <Util/HashName.h>
#include <Util/Macro.h>

#include <glad/vulkan.h>

namespace Render
{
   class Device
   {
   public:
      // Only need one instance
      static constexpr size_t MaxDeviceCount = 12u;
      CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Device, 1u, static_cast<uint32_t>(sizeof(Device)* MaxDeviceCount));

      struct Descriptor
      {
      };

      static eastl::unique_ptr<Device> CreateInstance(Descriptor&& p_desc)
      {
         return eastl::unique_ptr<Device>(new Device(eastl::move(p_desc)));
      }

      Device(Descriptor&& p_desc)
      {
         UNUSED(p_desc);
      }

   };
   //gladLoadVulkan();

}
