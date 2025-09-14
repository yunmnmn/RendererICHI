#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Vulkan/vulkan.hpp>

#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class RenderTypeToNative
{
 public:
   // Buffer usage to native type
   static VkBufferUsageFlags BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags);

   // MemoryProperty to native
   static VkMemoryPropertyFlags MemoryPropertyFlagsToNative(const MemoryPropertyFlags p_memoryPropertyFlags);

   // FrameBufferCreateFlags to native
   static VkFramebufferCreateFlagBits FrameBufferCreateFlagsToNative(const FrameBufferCreateFlags p_frameBufferCreateFlags);

   // CommandBufferPriority to native
   static VkCommandBufferLevel CommandBufferPriorityToNative(const CommandBufferPriority p_commandBufferPriority);

   // CommandBufferPriority to native
   static VkSemaphoreType SemaphoreTypeToNative(const SemaphoreType p_semaphoreType);

   // DescriptorType to native
   static VkDescriptorType DescriptorTypeToNative(const DescriptorType p_descriptorType);

   // ShaderStageFlag to native
   static VkShaderStageFlagBits ShaderStageFlagToNative(const ShaderStageFlag shaderStageFlag);

   static VkCullModeFlags CullModeToNative(const CullMode p_cullMode);

   static VkFrontFace FrontFaceToNative(const FrontFace p_frontFace);

   static VkPrimitiveTopology PrimitiveTopologyToNative(const PrimitiveTopology p_primitiveTopology);

   static VkPrimitiveTopology PrimitiveTopologyClassToNative(const PrimitiveTopologyClass p_primitiveTopologyClass);

   static VkCompareOp CompareOpToNative(const CompareOp p_compareOp);

   static VkStencilFaceFlags StencilFaceFlagsToNative(const StencilFaceFlags p_stencilFaceFlags);

   static VkStencilOp StencilOpToNative(const StencilOp p_stencilOp);

   static VkBlendFactor BlendFactorToNative(const BlendFactor p_blendFactor);

   static VkBlendOp BlendOpToNative(const BlendOp p_blendOp);

   static VkPipelineBindPoint PipelineBindPointToNative(const PipelineBindPoint p_pipelineBindPoint);

   static VkIndexType IndexTypeToNative(const IndexType p_indexType);

   static VkColorComponentFlagBits ColorComponentFlagsToNative(const ColorComponentFlags p_colorComponentFlags);

   static VkAttachmentLoadOp AttachmentLoadOpToNative(const AttachmentLoadOp p_attachmentLoadOp);

   static VkAttachmentStoreOp AttachmentStoreOpToNative(const AttachmentStoreOp p_attachmentStoreOp);

   static VkFormat ResourceFormatToNative(ResourceFormat p_format);

   static VkImageViewType ImageViewTypeToNative(ImageViewType p_viewType);

   static VkImageAspectFlagBits ImageAspectFlagsToNative(const ImageAspectFlags p_aspectFlags);
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
