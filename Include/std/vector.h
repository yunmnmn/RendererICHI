#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <std/Allocator.h>
#include <EASTL/vector.h>

namespace Render
{
   template<typename T>
   using vector = eastl::vector<T, RendererEastlAllocator>;
}
