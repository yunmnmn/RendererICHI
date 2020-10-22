#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Include/std/Allocator.h>
#include <EASTL/unordered_map.h>

namespace Render
{
   template <typename t_key, typename t_value>
   using unordered_map =
      eastl::unordered_map<t_key, t_value, eastl::hash<t_key>, eastl::equal_to<t_key>, RendererEastlAllocator, false>;
}
