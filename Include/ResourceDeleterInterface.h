#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

namespace Render
{

class Resource;

class ResourceDeleterInterface : public Foundation::Util::ManagerInterface<ResourceDeleterInterface>
{
 public:

   ResourceDeleterInterface() = default;
   ~ResourceDeleterInterface() = default;

 public:
   virtual void QueueForDeletion(Resource* p_resource) = 0;
   virtual void DeleteStaleResources(bool p_force = false) = 0;
};

} // namespace Render
