#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>
#include <ResourceReference.h>

#include <EASTL/weak_ptr.h>

namespace Render
{
class DescriptorPool;
class DescriptorPoolManagerInterface : public Foundation::Util::ManagerInterface<DescriptorPoolManagerInterface>
{
   friend DescriptorPool;

 public:
   // Each DescriptorPool has enough types available to allocate 12 instances of that particular DescriptorSet
   static constexpr uint32_t DescriptorSetInstanceCount = 12u;

   virtual ResourceUniqueRef<class DescriptorSet>
   AllocateDescriptorSet(ResourceRef<class DescriptorSetLayout> p_descriptorSetLayoutRef) = 0;

 private:
   // Queue the DescriptorPool for deletion
   virtual void QueueDescriptorPoolForDeletion(ResourceRef<DescriptorPool> p_descriptorPoolRef) = 0;
};
}; // namespace Render
