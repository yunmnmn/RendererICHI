#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <std/Allocator.h>
#include <EASTL/queue.h>
#include <std/deque.h>

namespace Render
{
template <typename T>
using queue = eastl::queue<T, Render::deque<T>>;
}
