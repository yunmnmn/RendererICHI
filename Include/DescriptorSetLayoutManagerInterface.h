#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Util/ManagerInterface.h>

#include <EASTL/sort.h>
#include <std/vector.h>

#include <ResourceReference.h>
#include <VulkanDevice.h>

#include <DescriptorSetLayout.h>

namespace Render
{
class VulkanDevice;

class DescriptorSetLayoutManagerInterface : public Foundation::Util::ManagerInterface<DescriptorSetLayoutManagerInterface>
{
 public:
   virtual ResourceRef<class DescriptorSetLayout> CreateOrGetDescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc) = 0;
};

}; // namespace Render
