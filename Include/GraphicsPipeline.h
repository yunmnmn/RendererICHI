#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <glm/glm.hpp>

namespace Render
{
class DescriptorSetLayout;
class VertexInputState;

enum class PrimitiveTopology : uint32_t
{
   PointList = 0u,
   LineList = 1u,
   LineStrip = 2u,
   TriangleList = 3u,
   TriangleStrip = 4u,
   TriangleFan = 5u,
   // TODO: add more
};

enum class PolygonMode : uint32_t
{
   PolygonModeFill = 0u,
   PolygonModeLine = 1u,
   PolygonModePoint = 2u,
};

enum class CullModeFlags : uint32_t
{
   CullModeNone = 0u,
   CullModeFront = 1u,
   CullModeBack = 2u,
   CullModeFrontAndBack = 3u,
};

enum class FrontFace : uint32_t
{
   FrontFaceCounterClockwise = 0u,
   FrontFaceClockwise = 1u,
};

struct Viewport
{
   float m_x = 0.0f;
   float m_y = 0.0f;
   float m_width = 0.0f;
   float m_height = 0.0f;
   // TODO: look into the depths members
   float m_minDepth = 0.0f;
   float m_maxDepth = 1.0f;
};

struct Scissors
{
   glm::ivec2 m_offset;
   glm::uvec2 m_extend;
};

struct RasterizationState
{
   bool m_depthClampEnable = false;
   bool m_rasterizationDiscard = false;
   PolygonMode m_polygonMode = PolygonMode::PolygonModeFill;
   CullModeFlags m_cullMode = CullModeFlags::CullModeBack;
   FrontFace m_frontFace = FrontFace::FrontFaceCounterClockwise;
   bool m_depthBiasEnable = false;
   float m_depthBiasConstantFactor = 0.0f;
   float m_depthBiasClamp = 0.0f;
   float m_depthBiasSlopeFactor = 0.0f;
   float m_lineWidth = 1.0f;
};

struct GraphicsPipelineDescriptor
{
   ResourceRef<DescriptorSetLayout> m_descriptorSetLayouts;
   ResourceRef<VertexInputState> m_vertexInputState;
   PrimitiveTopology m_primitiveTopology;
   RasterizationState m_rasterizationState;
   Scissors m_scissors;
   Viewport m_viewport;

   // TODO:
   // blend state
   // multi sample state
   // depth sample state
   // dynamic state
};

class GraphicsPipeline : public RenderResource<GraphicsPipeline>
{
 public:
   static constexpr size_t GraphicsPipelinePageCount = 12u;
   static constexpr size_t GraphicsPipelineCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(GraphicsPipeline, GraphicsPipelinePageCount,
                                      static_cast<uint32_t>(sizeof(GraphicsPipeline) * GraphicsPipelineCountPerPage));

   GraphicsPipeline() = delete;
   GraphicsPipeline(GraphicsPipelineDescriptor&& p_desc);
   ~GraphicsPipeline();

 private:
};
}; // namespace Render