#include <GHI/VertexInputState.h>

#include <Util/Util.h>

using namespace Foundation;

namespace Render
{

namespace GHI
{

VertexInputState::VertexInputState([[maybe_unused]] VertexInputStateDescriptor&& p_desc) : RenderResource(std::move(p_desc))
{
}

VertexInputState::~VertexInputState()
{
}

VertexInputBinding& VertexInputState::AddVertexInputBinding(VertexInputRate p_vertexInputRate)
{
   m_vertexInputBindings.emplace_back(p_vertexInputRate);
   return m_vertexInputBindings.back();
}

} // namespace GHI

} // namespace Render
