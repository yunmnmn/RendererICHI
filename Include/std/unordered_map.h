#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <std/Allocator.h>
#include <EASTL/unordered_map.h>

namespace Render
{
template <typename t_key, typename t_value, typename t_hash = eastl::hash<t_key>>
using unordered_map = eastl::unordered_map<t_key, t_value, t_hash, eastl::equal_to<t_key>, RendererEastlAllocator, false>;
}
