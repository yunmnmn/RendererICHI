#pragma once

#include <mutex>

#include <Std/unordered_set.h>

#include <Memory/AllocatorClass.h>

#include <ResourceTrackerInterface.h>

namespace Render
{

class Resource;

class ResourceTracker final : public ResourceTrackerInterface
{
 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ResourceTracker, 1u);

 public:
   ResourceTracker() = default;
   ~ResourceTracker();

 public:
   void Track(Resource* p_resource) final;
   void Untrack(Resource* p_resource) final;
   bool IsTracked(Resource* p_resource) final;

 private:
   std::recursive_mutex m_mutex;

   Std::unordered_set<Resource*> m_trackedResources;
};

} // namespace Render
