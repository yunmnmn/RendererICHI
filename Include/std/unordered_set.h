#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <std/Allocator.h>
#include <EASTL/unordered_set.h>

namespace Render
{
template <typename t_value>
using unordered_set = eastl::unordered_set<t_value, eastl::hash<t_value>, eastl::equal_to<t_value>, RendererEastlAllocator, false>;
}
