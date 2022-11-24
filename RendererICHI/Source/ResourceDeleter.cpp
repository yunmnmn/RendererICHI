#include <ResourceDeleter.h>

#include <RenderResource.h>
#include <RendererStateInterface.h>
#include <Renderer.h>

namespace Render
{

ResourceDeleter::~ResourceDeleter()
{
   DeleteStaleResources(true);
}

void ResourceDeleter::QueueForDeletion(Resource* p_resource)
{
   const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();

   std::lock_guard<std::recursive_mutex> lock(m_mutex);

   m_resourceDeleteContexts.emplace_back(p_resource, frameIndex);
}

void ResourceDeleter::DeleteStaleResources(bool p_force /*= false*/)
{
   const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();

   std::lock_guard<std::recursive_mutex> lock(m_mutex);

   if (p_force)
   {
      for (uint32_t i = 0u; i < m_resourceDeleteContexts.size(); i++)
      {

         delete m_resourceDeleteContexts[i].p_resource;
      }

      m_resourceDeleteContexts.clear();
   }
   else
   {
      for (uint32_t i = 0u; i < m_resourceDeleteContexts.size();)
      {
         if ((frameIndex - m_resourceDeleteContexts[i].m_frameIndex) >= RendererDefines::MaxQueuedFrames)
         {
            delete m_resourceDeleteContexts[i].p_resource;

            // Swap and pop
            eastl::iter_swap(m_resourceDeleteContexts.begin() + i, m_resourceDeleteContexts.end() - 1);
            m_resourceDeleteContexts.pop_back();
         }
         else
         {
            i++;
         }
      }
   }
}

} // namespace Render