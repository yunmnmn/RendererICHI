#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/BaseAllocator.h>
#include <Memory/StaticEastlAllocatorWrapper.h>
#include <Memory/TlsfSchema.h>

namespace Render
{
extern const char RenderEastlAllocatorName[];
using RendererEastlAllocator = Foundation::Memory::StaticEastlAllocatorWrapper<
    Foundation::Memory::DefaultAllocator<RenderEastlAllocatorName, Foundation::Memory::TlsfSchema<128u, 10240000>>>;
} // namespace Render
