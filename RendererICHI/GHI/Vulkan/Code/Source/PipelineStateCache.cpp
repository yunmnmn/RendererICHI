#include <GHI/Vulkan/PipelineStateCache.h>

#include <type_traits>

#include <Util/Assert.h>

#include <GHI/Vulkan/Device.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

namespace
{

template <class T>
void HashCombine(size_t& p_seed, const T& p_value)
{
   p_seed ^= std::hash<T>{}(p_value) + 0x9e3779b9u + (p_seed << 6u) + (p_seed >> 2u);
}

template <class E>
void HashEnum(size_t& p_seed, E p_value)
{
   using U = std::underlying_type_t<E>;
   HashCombine(p_seed, static_cast<U>(p_value));
}

} // namespace

bool GraphicsPipelineState::operator==(const GraphicsPipelineState& p_other) const
{
   return m_cullMode == p_other.m_cullMode && m_frontFace == p_other.m_frontFace &&
          m_primitiveTopology == p_other.m_primitiveTopology && m_viewportCount == p_other.m_viewportCount &&
          m_scissorCount == p_other.m_scissorCount && m_depthTestEnable == p_other.m_depthTestEnable &&
          m_depthWriteEnable == p_other.m_depthWriteEnable && m_depthCompareOp == p_other.m_depthCompareOp &&
          m_depthBoundsTestEnable == p_other.m_depthBoundsTestEnable &&
          m_stencilTestEnable == p_other.m_stencilTestEnable && m_stencilFaceMask == p_other.m_stencilFaceMask &&
          m_stencilFailOp == p_other.m_stencilFailOp && m_stencilPassOp == p_other.m_stencilPassOp &&
          m_stencilDepthFailOp == p_other.m_stencilDepthFailOp && m_stencilCompareOp == p_other.m_stencilCompareOp &&
          m_rasterizerDiscardEnable == p_other.m_rasterizerDiscardEnable &&
          m_depthBiasEnable == p_other.m_depthBiasEnable &&
          m_primitiveRestartEnable == p_other.m_primitiveRestartEnable && m_vertexStrides == p_other.m_vertexStrides;
}

size_t GraphicsPipelineStateHasher::operator()(const GraphicsPipelineState& p_state) const
{
   size_t seed = 0u;
   HashEnum(seed, p_state.m_cullMode);
   HashEnum(seed, p_state.m_frontFace);
   HashEnum(seed, p_state.m_primitiveTopology);
   HashCombine(seed, p_state.m_viewportCount);
   HashCombine(seed, p_state.m_scissorCount);
   HashCombine(seed, p_state.m_depthTestEnable);
   HashCombine(seed, p_state.m_depthWriteEnable);
   HashEnum(seed, p_state.m_depthCompareOp);
   HashCombine(seed, p_state.m_depthBoundsTestEnable);
   HashCombine(seed, p_state.m_stencilTestEnable);
   HashEnum(seed, p_state.m_stencilFaceMask);
   HashEnum(seed, p_state.m_stencilFailOp);
   HashEnum(seed, p_state.m_stencilPassOp);
   HashEnum(seed, p_state.m_stencilDepthFailOp);
   HashEnum(seed, p_state.m_stencilCompareOp);
   HashCombine(seed, p_state.m_rasterizerDiscardEnable);
   HashCombine(seed, p_state.m_depthBiasEnable);
   HashCombine(seed, p_state.m_primitiveRestartEnable);
   HashCombine(seed, p_state.m_vertexStrides.size());
   for (uint32_t stride : p_state.m_vertexStrides)
   {
      HashCombine(seed, stride);
   }
   return seed;
}

PipelineStateCache::PipelineStateCache(Ptr<Device> p_device)
{
   Init(std::move(p_device));
}

PipelineStateCache::~PipelineStateCache()
{
   Release();
}

void PipelineStateCache::Init(Ptr<Device> p_device)
{
   ASSERT(m_device == nullptr, "PipelineStateCache is already initialized");

   m_device = std::move(p_device);
}

void PipelineStateCache::Release()
{
   if (m_device == nullptr)
   {
      return;
   }

   for (const auto& [state, pipeline] : m_graphicsPipelines)
   {
      (void)state;
      vkDestroyPipeline(m_device->GetLogicalDeviceNative(), pipeline, nullptr);
   }
   m_graphicsPipelines.clear();

   m_device.reset();
}

VkPipeline PipelineStateCache::GetOrCreate(const GraphicsPipelineState& p_state, const CreatePipelineFn& p_createPipeline)
{
   const auto pipelineIt = m_graphicsPipelines.find(p_state);
   if (pipelineIt != m_graphicsPipelines.end())
   {
      return pipelineIt->second;
   }

   VkPipeline pipeline = p_createPipeline(p_state);
   m_graphicsPipelines.emplace(p_state, pipeline);
   return pipeline;
}

void PipelineStateCache::Prewarm(const GraphicsPipelineState& p_state, const CreatePipelineFn& p_createPipeline)
{
   (void)GetOrCreate(p_state, p_createPipeline);
}

} // namespace Vulkan
} // namespace GHI
} // namespace Render
