#pragma once

#include <GHI/Device.h>
#include <GHI/RenderGraph.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

void ConfigureRenderGraph(GHI::RenderGraph& p_renderGraph, Ptr<GHI::Device> p_device);

} // namespace Vulkan

} // namespace GHI

} // namespace Render
