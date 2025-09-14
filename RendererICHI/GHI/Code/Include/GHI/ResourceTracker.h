//#pragma once
//
//#include <mutex>
//#include <unordered_set>
//
//#include <GHI/ResourceTrackerInterface.h>
//#include <GHI/RenderResource.h>
//
//namespace Render
//{
//
//namespace GHI
//{
//
//class ResourceTracker final : public ResourceTrackerInterface
//{
// public:
//   ResourceTracker() = default;
//   ~ResourceTracker();
//
// public:
//   void Track(Resource* p_resource) final;
//   void Untrack(Resource* p_resource) final;
//   bool IsTracked(Resource* p_resource) final;
//
// private:
//   std::recursive_mutex m_mutex;
//
//   std::unordered_set<Resource*> m_trackedResources;
//};
//
//} // namespace GHI
//
//} // namespace Render
