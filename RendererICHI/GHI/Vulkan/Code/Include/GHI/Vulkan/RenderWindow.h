#pragma once

#include <GHI/RenderWindow.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class RenderWindow final : public GHI::RenderWindow
{
 public:
   RenderWindow() = delete;
   RenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_desc);
   ~RenderWindow() final;

 private:
   void DestroyWindow();

   ///////////////////////////////////////////////////
   // GHI::RenderWindow
   void PollEventsInternal() final;
   bool ShouldCloseInternal() const final;
   ///////////////////////////////////////////////////

   ///////////////////////////////////////////////////
   // GHI::Resource
   void ReleaseInternal() final;
   ///////////////////////////////////////////////////
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
