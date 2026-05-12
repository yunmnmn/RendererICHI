#pragma once

#include <glm/vec2.hpp>

#include <GHI/RendererTypes.h>

namespace Render::GHI
{
class CommandBuffer;
class ImageView;
class SubCommandRecorder;
}

namespace Render::GHI::Vulkan
{
class Device;
}

struct GLFWwindow;

namespace Render::Util
{

struct ImGuiContextDescriptor
{
   GLFWwindow* m_window = nullptr;

   // Must be a GHI::Vulkan::Device. VkPhysicalDevice is derived from it internally.
   GHI::Vulkan::Device* m_device = nullptr;

   // Swapchain color format used to configure the dynamic rendering pipeline.
   GHI::ResourceFormat m_swapchainColorFormat = GHI::ResourceFormat::Invalid;

   uint32_t m_imageCount = 0u;
   uint32_t m_minImageCount = 2u;
};

// Manages Dear ImGui lifecycle against the RendererICHI Vulkan backend.
// Uses dynamic rendering — no VkRenderPass required.
//
// Usage per frame:
//   1. NewFrame()                        — begin ImGui frame
//   2. ImGui::* calls                    — build UI
//   3. Render(cmd, extent, imageView)    — record draw commands via the GHI command buffer
class ImGuiContext
{
 public:
   ImGuiContext() = default;
   ~ImGuiContext();

   void Init(ImGuiContextDescriptor&& p_desc);
   void Shutdown();

   // Begin a new ImGui frame. Call before any ImGui::* UI calls.
   void NewFrame();

   // Finalize and inject ImGui draw commands into p_commandBuffer.
   // The target image must already be in ResourceUsage::ColorAttachmentWrite when the pass executes.
   // Load op is LOAD so existing scene content is preserved.
   void Render(GHI::CommandBuffer* p_commandBuffer, glm::uvec2 p_extent, GHI::ImageView* p_targetImageView);

   // Finalize and inject ImGui draw commands into an active graph-managed rendering scope.
   void Render(GHI::SubCommandRecorder* p_recorder);

 private:
   void* m_descriptorPool = nullptr; // VkDescriptorPool, stored opaque to keep Vulkan out of the header
   void* m_logicalDevice = nullptr;  // VkDevice
   bool m_initialized = false;
};

} // namespace Render::Util
