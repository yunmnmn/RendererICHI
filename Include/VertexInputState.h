#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Std/vector.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>
#include <RendererTypes.h>

using namespace Foundation;

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
   VertexInputBinding(VertexInputRate p_vertexInputRate)
   {
      m_vertexInputRate = p_vertexInputRate;
   }

   void AddVertexInputAttribute(uint32_t p_location, VkFormat p_format, uint32_t p_offset)
   {
      m_vertexInputAttributes.push_back(VertexInputAttribute{.m_location = p_location, .m_format = p_format, .m_offset = p_offset});
   }

 private:
   VertexInputRate m_vertexInputRate = VertexInputRate::VertexInputRateVertex;

   Std::vector<VertexInputAttribute> m_vertexInputAttributes;
};

struct VertexInputStateDescriptor
{
};

class VertexInputState : public RenderResource<VertexInputState>
{
 public:
   static constexpr size_t VertexInputStatePageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VertexInputState, VertexInputStatePageCount);

   VertexInputState() = delete;
   VertexInputState(VertexInputStateDescriptor&& p_desc);
   ~VertexInputState();

   // Add a VertexInputBinding
   VertexInputBinding& AddVertexInputBinding(VertexInputRate p_vertexInputRate);

   // Get the
   VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo();

 private:
   // Converts the Renderer's VertexInputRate to Vulkan's equivalent VKVertexInputRate
   const VkVertexInputRate VertexInputRateToNative(const VertexInputRate p_vertexInputRate) const;

   Std::vector<VertexInputBinding> m_vertexInputBindings;

   Std::vector<VkVertexInputBindingDescription> vertexInputBindingDescs;
   Std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescs;
};
}; // namespace Render
