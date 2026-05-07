#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

#include <GHI/RendererTypes.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

class Device;

struct GraphicsPipelineState
{
   CullMode m_cullMode = CullMode::CullModeNone;
   FrontFace m_frontFace = FrontFace::FrontFaceCounterClockwise;
   PrimitiveTopology m_primitiveTopology = PrimitiveTopology::TriangleList;
   uint32_t m_viewportCount = 1u;
   uint32_t m_scissorCount = 1u;
   bool m_depthTestEnable = false;
   bool m_depthWriteEnable = false;
   CompareOp m_depthCompareOp = CompareOp::Always;
   bool m_depthBoundsTestEnable = false;
   bool m_stencilTestEnable = false;
   StencilFaceFlags m_stencilFaceMask = StencilFaceFlags::FrontAndBack;
   StencilOp m_stencilFailOp = StencilOp::Keep;
   StencilOp m_stencilPassOp = StencilOp::Keep;
   StencilOp m_stencilDepthFailOp = StencilOp::Keep;
   CompareOp m_stencilCompareOp = CompareOp::Always;
   bool m_rasterizerDiscardEnable = false;
   bool m_depthBiasEnable = false;
   bool m_primitiveRestartEnable = false;
   std::vector<uint32_t> m_vertexStrides;

   bool operator==(const GraphicsPipelineState& p_other) const;
};

struct GraphicsPipelineStateHasher
{
   size_t operator()(const GraphicsPipelineState& p_state) const;
};

class PipelineStateCache final
{
 public:
   using CreatePipelineFn = std::function<VkPipeline(const GraphicsPipelineState&)>;

 public:
   PipelineStateCache() = default;
   explicit PipelineStateCache(Ptr<Device> p_device);
   ~PipelineStateCache();

   PipelineStateCache(const PipelineStateCache&) = delete;
   PipelineStateCache& operator=(const PipelineStateCache&) = delete;

 public:
   void Init(Ptr<Device> p_device);
   void Release();
   VkPipeline GetOrCreate(const GraphicsPipelineState& p_state, const CreatePipelineFn& p_createPipeline);
   void Prewarm(const GraphicsPipelineState& p_state, const CreatePipelineFn& p_createPipeline);

 private:
   Ptr<Device> m_device;
   std::unordered_map<GraphicsPipelineState, VkPipeline, GraphicsPipelineStateHasher> m_graphicsPipelines;
};

} // namespace Vulkan
} // namespace GHI
} // namespace Render
