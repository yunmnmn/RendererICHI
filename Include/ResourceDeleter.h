#pragma once

#include <mutex>

#include <Std/vector.h>

#include <Memory/AllocatorClass.h>

#include <ResourceDeleterInterface.h>

namespace Render
{

class Resource;

class ResourceDeleter final : public ResourceDeleterInterface
{
   struct ResourceDeleteContext
   {
      Resource* p_resource = nullptr;
      uint64_t m_frameIndex = 0ul;
   };

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ResourceDeleter, 1u);

 public:
   ResourceDeleter() = default;
   ~ResourceDeleter();

   void QueueForDeletion(Resource* p_resource) final;
   void DeleteStaleResources(bool p_force = false) final;

 private:
   Std::vector<ResourceDeleteContext> m_resourceDeleteContexts;
   std::recursive_mutex m_mutex;
};

} // namespace Render
