#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

namespace Render
{

class Resource;

class ResourceTrackerInterface : public Foundation::Util::ManagerInterface<ResourceTrackerInterface>
{
 public:
   ResourceTrackerInterface() = default;
   ~ResourceTrackerInterface() = default;

 public:
   virtual void Track(Resource* p_resource) = 0;
   virtual void Untrack(Resource* p_resource) = 0;
   virtual bool IsTracked(Resource* p_resource) = 0;
};

} // namespace Render
