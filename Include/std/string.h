#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <std/Allocator.h>
#include <EASTL/string.h>

namespace Render
{
using string = eastl::basic_string<char, RendererEastlAllocator>;

} // namespace Render
