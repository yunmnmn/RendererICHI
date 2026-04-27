#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/RenderResource.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

struct VertexInputAttribute
{
   uint32_t m_location = 0u;
   ResourceFormat m_format;
   uint32_t m_offset = 0u;
};

struct VertexInputBinding
{
   VertexInputBinding() = delete;
   VertexInputBinding(VertexInputRate p_vertexInputRate)
   {
      m_vertexInputRate = p_vertexInputRate;
   }

   void AddVertexInputAttribute(uint32_t p_location, ResourceFormat p_format, uint32_t p_offset)
   {
      m_vertexInputAttributes.push_back(VertexInputAttribute{.m_location = p_location, .m_format = p_format, .m_offset = p_offset});
   }

   VertexInputRate m_vertexInputRate = VertexInputRate::VertexInputRateVertex;
   uint32_t m_stride = 0u;
   std::vector<VertexInputAttribute> m_vertexInputAttributes;
};

struct VertexInputStateDescriptor
{
};

class VertexInputState : public RenderResource<VertexInputStateDescriptor>
{
 protected:
   VertexInputState(VertexInputStateDescriptor&& p_desc);

 public:
   virtual ~VertexInputState() = 0;

 public:
   VertexInputBinding& AddVertexInputBinding(VertexInputRate p_vertexInputRate);

 protected:
   std::vector<VertexInputBinding> m_vertexInputBindings;
};

} // namespace GHI

}; // namespace Render
