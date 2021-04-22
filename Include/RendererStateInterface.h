#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

#include <ResourceReference.h>

namespace Render
{
class RenderStateInterface : public Foundation::Util::ManagerInterface<RenderStateInterface>
{
 public:
   virtual void IncrementFrameIndex() = 0;
   virtual uint64_t GetFrameIndex() const = 0;
   virtual uint32_t GetResourceIndex() const = 0;

 private:
};
}; // namespace Render
