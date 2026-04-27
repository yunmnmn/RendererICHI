#include <Module/Module.h>

#include <GHI/Vulkan/ResourceFactory.h>
#include <GHI/Vulkan/VulkanInstance.h>

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
 public:
   VulkanModule()
   {
   }

 private:
   void OnLoad() final
   {
      GHI::ResourceFactory::Register(&g_factory);

      VulkanInstance::Get()->Init({});
   }

   void OnUnload() final
   {
      VulkanInstance::Get()->Shutdown();

      GHI::ResourceFactory::Unregister();
   }
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render

#undef ASSERT
#define ASSERT(Expr, Msg) _assert(#Expr, static_cast<bool>(Expr), __FILE__, __LINE__, Msg)

#pragma warning(disable : 5205)

// Define the entrypoint for the module
MODULE_ENTRY_POINT(Render::GHI::Vulkan::VulkanModule);
