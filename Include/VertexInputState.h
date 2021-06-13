#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <std/vector.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <RendererTypes.h>

namespace Render
{
enum class VertexInputRate : uint32_t
{
   VertexInputRateVertex = 0u,
   VertexInputRateInstance = 1u,
};

struct VertexInputAttribute
{
   uint32_t m_location = 0u;
   VkFormat m_format;
   uint32_t m_offset = 0u;
};

struct VertexInputBinding
{
   friend class VertexInputState;

   VertexInputBinding() = delete;
   VertexInputBinding(VertexInputRate p_vertexInputRate, uint32_t p_stride)
   {
      m_vertexInputRate = p_vertexInputRate;
      m_stride = p_stride;
   }

   void AddVertexInputAttribute(uint32_t p_location, VkFormat p_format, uint32_t p_offset)
   {
      m_vertexInputAttributes.push_back(VertexInputAttribute{.m_location = p_location, .m_format = p_format, .m_offset = p_offset});
   }

 private:
   VertexInputRate m_vertexInputRate = VertexInputRate::VertexInputRateVertex;
   uint32_t m_stride = 0u;

   Render::vector<VertexInputAttribute> m_vertexInputAttributes;
};

struct VertexInputStateDescriptor
{
};

class VertexInputState : public RenderResource<VertexInputState>
{
 public:
   static constexpr size_t VertexInputStatePageCount = 12u;
   static constexpr size_t VertexInputStateCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VertexInputState, VertexInputStatePageCount,
                                      static_cast<uint32_t>(sizeof(VertexInputState) * VertexInputStateCountPerPage));

   VertexInputState() = delete;
   VertexInputState(VertexInputStateDescriptor&& p_desc);
   ~VertexInputState();

   // Add a VertexInputBinding
   VertexInputBinding& AddVertexInputBinding(VertexInputRate p_vertexInputRate, uint32_t p_stride);

   // Get the
   VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo();

 private:
   // Converts the Renderer's VertexInputRate to Vulkan's equivalent VKVertexInputRate
   const VkVertexInputRate VertexInputRateToNative(const VertexInputRate p_vertexInputRate) const;

   Render::vector<VertexInputBinding> m_vertexInputBindings;

   Render::vector<VkVertexInputBindingDescription> vertexInputBindingDescs;
   Render::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescs;
};
}; // namespace Render
