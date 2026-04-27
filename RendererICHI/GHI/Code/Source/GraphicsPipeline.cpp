#include <GHI/GraphicsPipeline.h>

#include <Util/Util.h>

namespace Render
{

namespace GHI
{

GraphicsPipeline::GraphicsPipeline(Ptr<Device> p_device, GraphicsPipelineDescriptor&& p_desc)
    : DeviceResource<GraphicsPipelineDescriptor>(p_device, std::move(p_desc))
{
}

GraphicsPipeline::~GraphicsPipeline()
{
}

} // namespace GHI

} // namespace Render
