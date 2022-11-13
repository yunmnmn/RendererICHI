#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

#include <vulkan/vulkan.h>

#include <ResourceReference.h>
#include <RendererTypes.h>

namespace Render
{

class CommandBuffer;

class CommandPoolManagerInterface : public Foundation::Util::ManagerInterface<CommandPoolManagerInterface>
{
 protected:
   CommandPoolManagerInterface() = default;
   virtual ~CommandPoolManagerInterface() = default;

   virtual void CompileCommandBuffer(ResourceRef<CommandBuffer> p_commandBuffer) = 0;
};

}; // namespace Render
