#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>
#include <RenderResource.h>

namespace Render
{

class DescriptorPool;
class DescriptorSet;

class DescriptorPoolManagerInterface : public Foundation::Util::ManagerInterface<DescriptorPoolManagerInterface>
{
   friend DescriptorPool;

 public:
   // Each DescriptorPool has enough types available to allocate 12 instances of that particular DescriptorSet
   static constexpr uint32_t DescriptorSetInstanceCount = 12u;

   virtual void AllocateDescriptorSet(DescriptorSet* p_descriptorSet) = 0;

 private:
};

}; // namespace Render
