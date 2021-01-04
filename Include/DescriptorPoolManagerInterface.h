#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

#include <EASTL/weak_ptr.h>

namespace Render
{
class DescriptorPoolManagerInterface : public Foundation::Util::ManagerInterface<DescriptorPoolManagerInterface>
{
 public:
   // Each DescriptorPool has enough types available to allocate 12 instances of that particular DescriptorSet
   static constexpr uint32_t DescriptorSetInstanceCount = 12u;

   virtual eastl::unique_ptr<class DescriptorSet>
   AllocateDescriptorSet(eastl::weak_ptr<class DescriptorSetLayout*> p_descriptorSetLayoutRef) = 0;
};
}; // namespace Render
