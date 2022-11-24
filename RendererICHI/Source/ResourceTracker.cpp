#include <ResourceTracker.h>

#include <RenderResource.h>

namespace Render
{

ResourceTracker::~ResourceTracker()
{
   ASSERT(m_trackedResources.empty(), "There are dangling resources");
}

void ResourceTracker::Track(Resource* p_resource)
{
   std::lock_guard<std::recursive_mutex> lock(m_mutex);

   ASSERT(IsTracked(p_resource) == false, "Resource was already registered");
   m_trackedResources.insert(p_resource);
}

void ResourceTracker::Untrack(Resource* p_resource)
{
   std::lock_guard<std::recursive_mutex> lock(m_mutex);

   ASSERT(IsTracked(p_resource) == true, "Resource was never registered");
   m_trackedResources.erase(p_resource);
}

bool ResourceTracker::IsTracked(Resource* p_resource)
{
   return m_trackedResources.find(p_resource) != m_trackedResources.end();
}

} // namespace Render