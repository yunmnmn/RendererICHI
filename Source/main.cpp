#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>

#include <Util/HashName.h>
#include <Memory/ClassAllocator.h>

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

   eastl::unique_ptr<Render::RenderInstance> renderInstance =
       Render::RenderInstance::CreateInstance({.m_instanceName = "Renderer", .m_version = 0u});

   // renderInstance->AddExtension({VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
   // "VK_LAYER_KHRONOS_validation"});
   renderInstance->AddExtension({Foundation::Util::HashName("test")});

   return 0;
}
