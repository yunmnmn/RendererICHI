#include <Module/Module.h>

#include <GHI/ResourceFactory.h>

using namespace Foundation;

namespace Render
{

namespace GHI
{

namespace Vulkan
{

Vulkan::ResourceFactory g_factory;

class VulkanModule final : public Foundation::Module
{
   VulkanModule()
   {
   }

 private:
   void OnLoad() final
   {
      GHI::ResourceFactory::Register(&g_factory);

      VulkanInstance::Get::Init();
   }

   void OnUnload() final
   {
      VulkanInstance::Get::Shutdown();

      GHI::ResourceFactory::Unregister();
   }
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render

// Define the entrypoint for the module
MODULE_ENTRY_POINT(Render::GHI::Vulkan::VulkanModule);
