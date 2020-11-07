#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

#include <EASTL/shared_ptr.h>
#include <std/vector.h>

#include <glad/vulkan.h>

namespace Render
{
struct DescriptorSetlayoutDescriptor
{
   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
};

class DescriptorSetLayoutManagerInterface : public Foundation::Util::ManagerInterface<DescriptorSetLayoutManagerInterface>
{
 public:
   virtual class DescriptorSetLayout* CreateOrGetDescriptorSetLayout(DescriptorSetlayoutDescriptor&& p_desc) = 0;
};

}; // namespace Render
