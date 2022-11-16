#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>
#include <Util/HashName.h>
#include <RenderResource.h>

#include <vulkan/vulkan.h>

namespace Render
{
class VulkanInstanceInterface : public Foundation::Util::ManagerInterface<VulkanInstanceInterface>
{
 public:
   virtual VkInstance GetInstanceNative() const = 0;
   virtual bool IsLayerUsed(Foundation::Util::HashName layerName) const = 0;
   virtual bool IsExtensionUsed(Foundation::Util::HashName extensionName) const = 0;

   // virtual Ptr<class VulkanDevice> GetSelectedVulkanDevice() = 0;
};
}; // namespace Render
