#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/shared_ptr.h>
#include <EASTL/type_traits.h>

#include <std/Allocator.h>

namespace Render
{
template <typename T>
struct RendererDeleter
{
   RendererDeleter() = default;

   template <typename U> // Enable if T* can be constructed with U* (i.e. U* is convertible to T*).
   RendererDeleter(const RendererDeleter<U>&, typename eastl::enable_if<eastl::is_convertible<U*, T*>::value>::type* = 0)
   {
   }

   void operator()(T* p) const
   {
      RendererEastlAllocator::deallocate(static_cast<void*>(p), 0u);
   }
};

template <typename T>
using shared_ptr = eastl::shared_ptr<T>;
} // namespace Render
