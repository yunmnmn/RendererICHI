#include <Util/ImGui/ImGuiContext.h>

#include <cassert>
#include <iterator>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <GHI/CommandBuffer.h>
#include <GHI/ImageView.h>

#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/ImageView.h>
#include <GHI/Vulkan/PhysicalDevice.h>
#include <GHI/Vulkan/RendererTypes.h>
#include <GHI/Vulkan/VulkanInstance.h>

namespace Render::Util
{

ImGuiContext::~ImGuiContext()
{
   if (m_initialized)
   {
      Shutdown();
   }
}

void ImGuiContext::Init(ImGuiContextDescriptor&& p_desc)
{
   assert(p_desc.m_window != nullptr);
   assert(p_desc.m_device != nullptr);
   assert(p_desc.m_swapchainColorFormat != GHI::ResourceFormat::Invalid);
   assert(p_desc.m_imageCount > 0u);

   VkDevice logicalDevice = p_desc.m_device->GetLogicalDeviceNative();
   m_logicalDevice = logicalDevice;

   auto* vulkanPhysDevice =
       static_cast<GHI::Vulkan::PhysicalDevice*>(p_desc.m_device->GetPhysicalDevice().get());

   VkDescriptorPoolSize poolSizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER,                1},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1},
   };

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
   poolInfo.maxSets = 3;
   poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
   poolInfo.pPoolSizes = poolSizes;

   VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
   vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool);
   m_descriptorPool = descriptorPool;

   IMGUI_CHECKVERSION();
   ::ImGui::CreateContext();

   ImGuiIO& io = ::ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

   ImGui_ImplGlfw_InitForVulkan(p_desc.m_window, true);

   VkFormat colorFormat = GHI::Vulkan::RenderTypeToNative::ResourceFormatToNative(p_desc.m_swapchainColorFormat);

   VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo{};
   pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
   pipelineRenderingInfo.colorAttachmentCount = 1;
   pipelineRenderingInfo.pColorAttachmentFormats = &colorFormat;

   ImGui_ImplVulkan_InitInfo initInfo{};
   initInfo.ApiVersion = VK_API_VERSION_1_3;
   initInfo.Instance = GHI::Vulkan::VulkanInstance::Get()->GetInstanceNative();
   initInfo.PhysicalDevice = vulkanPhysDevice->GetPhysicalDeviceNative();
   initInfo.Device = logicalDevice;
   initInfo.QueueFamily = p_desc.m_device->GetGraphicsQueueFamilyIndex();
   initInfo.Queue = p_desc.m_device->GetGraphicsQueueNative();
   initInfo.DescriptorPool = descriptorPool;
   initInfo.MinImageCount = p_desc.m_minImageCount;
   initInfo.ImageCount = p_desc.m_imageCount;
   initInfo.UseDynamicRendering = true;
   initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;

   ImGui_ImplVulkan_Init(&initInfo);

   m_initialized = true;
}

void ImGuiContext::Shutdown()
{
   vkDeviceWaitIdle(static_cast<VkDevice>(m_logicalDevice));

   ImGui_ImplVulkan_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ::ImGui::DestroyContext();

   vkDestroyDescriptorPool(static_cast<VkDevice>(m_logicalDevice),
                           static_cast<VkDescriptorPool>(m_descriptorPool), nullptr);
   m_descriptorPool = nullptr;
   m_initialized = false;
}

void ImGuiContext::NewFrame()
{
   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ::ImGui::NewFrame();
}

void ImGuiContext::Render(GHI::CommandBuffer* p_commandBuffer, glm::uvec2 p_extent, GHI::ImageView* p_targetImageView)
{
   ::ImGui::Render();
   ImDrawData* drawData = ::ImGui::GetDrawData();

   auto* vulkanImageView = static_cast<GHI::Vulkan::ImageView*>(p_targetImageView);
   VkImageView nativeImageView = vulkanImageView->GetImageViewNative();

   const VkExtent2D extent{p_extent.x, p_extent.y};

   p_commandBuffer->ExecuteRawRenderAPICallback([drawData, nativeImageView, extent](void* p_nativeCommandBuffer)
   {
      auto commandBuffer = static_cast<VkCommandBuffer>(p_nativeCommandBuffer);

      VkRenderingAttachmentInfo colorAttachment{};
      colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
      colorAttachment.imageView = nativeImageView;
      colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
      colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

      VkRenderingInfo renderingInfo{};
      renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
      renderingInfo.renderArea.extent = extent;
      renderingInfo.layerCount = 1;
      renderingInfo.colorAttachmentCount = 1;
      renderingInfo.pColorAttachments = &colorAttachment;

      vkCmdBeginRendering(commandBuffer, &renderingInfo);
      ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
      vkCmdEndRendering(commandBuffer);
   });
}

} // namespace Render::Util
