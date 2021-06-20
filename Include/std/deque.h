#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <std/Allocator.h>
#include <EASTL/deque.h>

namespace Render
{
template <typename T>
using deque = eastl::deque<T, RendererEastlAllocator>;
}
