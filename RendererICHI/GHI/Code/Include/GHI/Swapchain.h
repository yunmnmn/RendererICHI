#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <span>

#include <glfw/glfw3.h>

#include <GHI/DeviceResource.h>
#include <GHI/RenderWindow.h>
#include <GHI/ImageView.h>

namespace Render
{

namespace GHI
{

struct SwapchainDescriptor
{
   Ptr<RenderWindow> m_renderWindow;
};

class Swapchain : public DeviceResource<SwapchainDescriptor>
{
 protected:
   Swapchain(Ptr<Device> p_device, SwapchainDescriptor&& p_desc);

 public:
   virtual ~Swapchain() = 0;

   void Init();

 public:
   uint32_t GetSwapchainImageCount() const;
   glm::uvec2 GetExtend() const;
   ResourceFormat GetFormat() const;

   std::span<Ptr<Image>> GetSwapchainImages();
   std::span<Ptr<ImageView>> GetSwapchainImageViews();

 private:
   virtual void InitInternal() = 0;

 protected:
   std::vector<Ptr<Image>> m_swapchainImages;

   uint32_t m_swapchainCount = 0u;
   glm::uvec2 m_swapchainExtent = {};
   ResourceFormat m_swapchainFormat = ResourceFormat::Invalid;

 private:
   std::vector<Ptr<ImageView>> m_swapchainImageViews;
};

} // namespace GHI

} // namespace Render
