#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

namespace Render
{
class DescriptorPoolManagerInterface : public Foundation::Util::ManagerInterface<DescriptorPoolManagerInterface>
{
 public:
   virtual eastl::unique_ptr<class DescriptorSet> AllocateDescriptorSet(class DescriptorSetLayout* p_descriptorSetLayout) = 0;
};
}; // namespace Render
