#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <std/Allocator.h>
#include <EASTL/list.h>

namespace Render
{
template <typename T>
using list = eastl::list<T, RendererEastlAllocator>;
}
