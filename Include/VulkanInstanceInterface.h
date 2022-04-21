#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>
#include <Util/HashName.h>
#include <ResourceReference.h>

#include <glad/vulkan.h>

namespace Render
{
class VulkanInstanceInterface : public Foundation::Util::ManagerInterface<VulkanInstanceInterface>
{
 public:
   virtual VkInstance GetInstanceNative() const = 0;
   virtual bool IsLayerUsed(Foundation::Util::HashName layerName) const = 0;
   virtual bool IsExtensionUsed(Foundation::Util::HashName extensionName) const = 0;

   // virtual ResourceRef<class VulkanDevice> GetSelectedVulkanDevice() = 0;
};
}; // namespace Render
