#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

namespace Render
{

enum class ResourceFormat : uint32_t
{
   Undefined = 0u,
   R4G4UnormPack8 = 1u,
   R4G4B4A4UnormPack16 = 2u,
   B4G4R4A4UnormPack16 = 3u,
   R5G6B5UnormPack16 = 4u,
   B5G6R5UnormPack16 = 5u,
   R5G5B5A1UnormPack16 = 6u,
   B5G5R5A1UnormPack16 = 7u,
   A1R5G5B5UnormPack16 = 8u,
   R8Unorm = 9u,
   R8Snorm = 10u,
   R8Scaled = 11u,
   R8SScaled = 12u,
   R8Uint = 13u,
   R8Sint = 14u,
   R8Srgb = 15u,
   R8G8Unorm = 16u,
   R8G8Snorm = 17u,
   R8G8Uscaled = 18u,
   R8G8Sscaled = 19u,
   R8G8Uint = 20u,
   // TODO: more
};

enum class QueueFamilyTypeFlags : uint32_t
{
   GraphicsQueue = (1 << 0),
   ComputeQueue = (1 << 1),
   TransferQueue = (1 << 2),

   AllQueues = GraphicsQueue + ComputeQueue + TransferQueue,
};

enum class QueueFamilyType : uint32_t
{
   GraphicsQueue = 0u,
   ComputeQueue,
   TransferQueue,

   Count,
   Invalid = Count
};

enum class MemoryPropertyFlags : uint32_t
{
   DeviceLocal = (1 << 0),
   HostVisible = (1 << 1),
   HostCoherent = (1 << 2),
   HostCached = (1 << 3),
};

enum class DescriptorType : uint32_t
{
   Sampler,
   CombinedImageSampler,
   SampledImage,
   StorageImage,
   UniformTexelBuffer,
   StorageTexelBuffer,
   UniformBuffer,
   StorageBuffer,
   InputAttachment,
   // TODO: Add support for Inline uniform block?
   // TODO: Add support for acceleration structures if I ever get hold of a RTX card :')

   Count,
   Invalid = Count
};

enum class ShaderStageFlag : uint32_t
{
   Vertex = (1 << 0),
   Fragment = (1 << 1),
   Compute = (1 << 2),

   All = Vertex | Fragment | Compute
};

enum class BufferUsageFlags : uint32_t
{
   TransferSource = (1 << 0),
   TransferDestination = (1 << 1),
   UniformTexel = (1 << 2),
   StorageTexel = (1 << 3),
   Uniform = (1 << 4),
   Storage = (1 << 5),
   IndexBuffer = (1 << 6),
   VertexBuffer = (1 << 7),
   IndirectBuffer = (1 << 8),
};

enum class BufferUsage : uint32_t
{
   TransferSource = 0u,
   TransferDestination,
   UniformTexel,
   StorageTexel,
   Uniform,
   Storage,
   IndexBuffer,
   VertexBuffer,
   IndirectBuffer,

   Count,
   Invalid = Count
};

enum class BlendFactor : uint32_t
{
   FactorZero,
   FactorOne,
   SrcColor,
   OneMinusSrcColor,
   DstColor,
   OneMinusDstColor,
   SrcAlpha,
   OneMinusSrcAlpha,
   DstAlpha,
   OneMinusDstAlpha,
   ConstantColor,
   OneMinusConstantColor,
   ConstantAlpha,
   OneMinusConstantAlpha,
   SrcAlphaSaturate,
   Src1Color,
   OneMinusSrc1Color,
   Src1Alpha,
   OneMinusSrc1Alpha,

   Count,
   Invalid = Count
};

enum class BlendOp : uint32_t
{
   Add,
   Subtract,
   ReverseSubtract,
   Min,
   Max,

   Count,
   Invalid = Count
};

enum class CompareOp : uint32_t
{
   Never,
   Less,
   Equal,
   LessOrEqual,
   Greater,
   NotEqual,
   GreaterOrEqual,
   Always,

   Count,
   Invalid = Count
};

enum class StencilOp : uint32_t
{
   Keep,
   Zero,
   Replace,
   IntrecmentAndClamp,
   DecrementAndClamp,
   Invert,
   IncrementAndWrap,
   DecrementAndWrap,

   Count,
   Invalid = Count
};

enum class StencilFaceFlags : uint32_t
{
   None = 0u,
   Front = (1u << 0),
   Back = (1u << 1),

   FrontAndBack = Front | Back,
};

enum class FrameBufferCreateFlags : uint32_t
{
   CreateImageless = (1u << 0),
};

enum class CommandBufferPriority : uint32_t
{
   Primary = 0u,
   Secondary = 1u,

   Count,
   Invalid = Count,
};

enum class SemaphoreType : uint32_t
{
   Binary = 0u,
   Timeline,

   Count,
   Invalid = Count,
};

enum class CullMode : uint32_t
{
   CullModeNone = 0u,
   CullModeFront,
   CullModeBack,
   CullModeFrontAndBack,

   Count,
   Invalid = Count
};

enum class PrimitiveTopologyClass : uint32_t
{
   Point = 0u,
   Line,
   Triangle,

   Count,
   Invalid = Count
};

enum class PrimitiveTopology : uint32_t
{
   PointList = 0u,
   LineList,
   LineStrip,
   TriangleList,
   TriangleStrip,
   TriangleFan,
   // TODO: add more

   Count,
   Invalid = Count
};

enum class FrontFace : uint32_t
{
   FrontFaceCounterClockwise = 0u,
   FrontFaceClockwise,

   Count,
   Invalid = Count
};

enum class ColorComponentFlags : uint32_t
{
   R = (1 << 0),
   G = (1 << 1),
   B = (1 << 2),
   A = (1 << 3),

   RGBA = R | G | B | A
};

enum class PipelineBindPoint : uint32_t
{
   Graphics = 0u,
   Compute,

   Count,
   Invalid = Count
};

enum class IndexType : uint32_t
{
   Uint16,
   Uint32,

   Count,
   Invalid = Count
};

enum class SrcStageMask : uint32_t
{
   // TODO
};

enum class RenderCommandType : uint32_t
{
   SetState,
   Action,
   ExecuteCommand,
   BeginRender,
   EndRender,
   Barrier,

   Count,
   Invalid = Count
};

enum class PolygonMode : uint32_t
{
   PolygonModeFill = 0u,
   PolygonModeLine,
   PolygonModePoint,

   Count,
   Invalid = Count
};

enum class AttachmentLoadOp : uint32_t
{
   Load,
   Clear,
   DontCare,

   Count,
   Invalid = Count
};

enum class AttachmentStoreOp : uint32_t
{
   Store,
   DontCare,

   Count,
   Invalid = Count
};

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
};

}; // namespace Render
