#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>

#include <Util/HashName.h>
#include <Memory/ClassAllocator.h>

#include <Logger.h>

#include <RenderInstance.h>
#include <GLFW/glfw3.h>

int main()
{
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);

   if (!glfwInit())
   {
      ASSERT(false, "Failed to initialize glfw");
   }

   // Create a Vulkan instance
   Render::RenderInstance::Descriptor descriptor{
       .m_instanceName = "Renderer",
       .m_version = 0u,
       .m_layers = {"VK_LAYER_KHRONOS_validation"},
       .m_extensions = {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME}};
   eastl::unique_ptr<Render::RenderInstance> renderInstance = Render::RenderInstance::CreateInstance(eastl::move(descriptor));

   return 0;
}
