#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

namespace Render
{
class DescriptorPoolManagerInterface : public Foundation::Util::ManagerInterface<DescriptorPoolManagerInterface>
{
 public:
};
}; // namespace Render
