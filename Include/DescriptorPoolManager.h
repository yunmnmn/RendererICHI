#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <Memory/ClassAllocator.h>

#include <DescriptorPoolManagerInterface.h>

namespace Render
{
class DescriptorPoolManager : public DescriptorPoolManagerInterface
{
 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPoolManager, 1u, static_cast<uint32_t>(sizeof(DescriptorPoolManager)));

   DescriptorPoolManager()
   {
   }

   ~DescriptorPoolManager()
   {
   }

 private:
   std::mutex m_descriptorPoolManagerMutex;
};
}; // namespace Render
