#include <GHI/Vulkan/RendererTypes.h>

#include <unordered_map>

#include <Util/Util.h>

#include <GHI/Renderer.h>

using namespace Foundation;

namespace Render
{
namespace GHI
{
namespace Vulkan
{

VkBufferUsageFlags RenderTypeToNative::BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags)
{
   static const std::unordered_map<BufferUsageFlags, VkBufferUsageFlags> BufferUsageFlagsToNativeMap = {
       {BufferUsageFlags::TransferSource, VK_BUFFER_USAGE_TRANSFER_SRC_BIT},
       {BufferUsageFlags::TransferDestination, VK_BUFFER_USAGE_TRANSFER_DST_BIT},
       {BufferUsageFlags::UniformTexel, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT},
       {BufferUsageFlags::StorageTexel, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT},
       {BufferUsageFlags::Uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
       {BufferUsageFlags::Storage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
       {BufferUsageFlags::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
       {BufferUsageFlags::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
       {BufferUsageFlags::IndirectBuffer, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkBufferUsageFlags>(BufferUsageFlagsToNativeMap, p_bufferUsageFlags);
}

VkMemoryPropertyFlags RenderTypeToNative::MemoryPropertyFlagsToNative(const MemoryPropertyFlags p_memoryPropertyFlags)
{
   static const std::unordered_map<MemoryPropertyFlags, VkMemoryPropertyFlags> MemoryPropertyFlagsToNativeMap = {
       {MemoryPropertyFlags::DeviceLocal, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
       {MemoryPropertyFlags::HostVisible, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT},
       {MemoryPropertyFlags::HostCoherent, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
       {MemoryPropertyFlags::HostCached, VK_MEMORY_PROPERTY_HOST_CACHED_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkMemoryPropertyFlags>(MemoryPropertyFlagsToNativeMap, p_memoryPropertyFlags);
}

VkFramebufferCreateFlagBits
RenderTypeToNative::FrameBufferCreateFlagsToNative(const FrameBufferCreateFlags p_frameBufferCreateFlags)
{
   static const std::unordered_map<FrameBufferCreateFlags, VkFramebufferCreateFlagBits> FrameBufferCreateFlagsToNativeMap = {
       {FrameBufferCreateFlags::CreateImageless, VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkFramebufferCreateFlagBits>(FrameBufferCreateFlagsToNativeMap,
                                                                             p_frameBufferCreateFlags);
}

VkCommandBufferLevel RenderTypeToNative::CommandBufferPriorityToNative(const CommandBufferPriority p_commandBufferPriority)
{
   static const std::unordered_map<CommandBufferPriority, VkCommandBufferLevel> CommandBufferPriorityToNativeMap = {
       {CommandBufferPriority::Primary, VK_COMMAND_BUFFER_LEVEL_PRIMARY},
       {CommandBufferPriority::Secondary, VK_COMMAND_BUFFER_LEVEL_SECONDARY},
   };

   return Foundation::Util::EnumToNativeHelper<VkCommandBufferLevel>(CommandBufferPriorityToNativeMap, p_commandBufferPriority);
}

VkSemaphoreType RenderTypeToNative::SemaphoreTypeToNative(const SemaphoreType p_semaphoreType)
{
   static const std::unordered_map<SemaphoreType, VkSemaphoreType> SemaphoreTypeToNativeMap = {
       {SemaphoreType::Binary, VK_SEMAPHORE_TYPE_BINARY},
       {SemaphoreType::Timeline, VK_SEMAPHORE_TYPE_TIMELINE},
   };

   return Foundation::Util::EnumToNativeHelper<VkSemaphoreType>(SemaphoreTypeToNativeMap, p_semaphoreType);
}

VkDescriptorType RenderTypeToNative::DescriptorTypeToNative(const DescriptorType p_descriptorType)
{
   static const std::unordered_map<DescriptorType, VkDescriptorType> DescriptorTypeToNativeMap = {
       {DescriptorType::Sampler, VK_DESCRIPTOR_TYPE_SAMPLER},
       {DescriptorType::CombinedImageSampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
       {DescriptorType::SampledImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
       {DescriptorType::StorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
       {DescriptorType::UniformTexelBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
       {DescriptorType::StorageTexelBuffer, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
       {DescriptorType::UniformBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
       {DescriptorType::StorageBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC},
       {DescriptorType::InputAttachment, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT},
   };

   return Foundation::Util::EnumToNativeHelper<VkDescriptorType>(DescriptorTypeToNativeMap, p_descriptorType);
}

VkShaderStageFlagBits RenderTypeToNative::ShaderStageFlagToNative(const ShaderStageFlag shaderStageFlag)
{
   static const std::unordered_map<ShaderStageFlag, VkShaderStageFlagBits> ShaderStageFlagToNativeMap = {
       {ShaderStageFlag::Vertex, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT},
       {ShaderStageFlag::Fragment, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT},
       {ShaderStageFlag::Compute, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkShaderStageFlagBits>(ShaderStageFlagToNativeMap, shaderStageFlag);
}

VkCullModeFlags RenderTypeToNative::CullModeToNative(const CullMode p_cullMode)
{
   static const std::unordered_map<CullMode, VkCullModeFlags> CullModeToNativeMap = {
       {CullMode::CullModeNone, VK_CULL_MODE_NONE},
       {CullMode::CullModeFront, VK_CULL_MODE_FRONT_BIT},
       {CullMode::CullModeBack, VK_CULL_MODE_BACK_BIT},
       {CullMode::CullModeFrontAndBack, VK_CULL_MODE_FRONT_AND_BACK},
   };

   return Foundation::Util::EnumToNativeHelper<VkCullModeFlags>(CullModeToNativeMap, p_cullMode);
}

VkFrontFace RenderTypeToNative::FrontFaceToNative(const FrontFace p_frontFace)
{
   static const std::unordered_map<FrontFace, VkFrontFace> FrontFaceToNativeMap = {
       {FrontFace::FrontFaceCounterClockwise, VK_FRONT_FACE_COUNTER_CLOCKWISE},
       {FrontFace::FrontFaceClockwise, VK_FRONT_FACE_CLOCKWISE},
   };

   return Foundation::Util::EnumToNativeHelper<VkFrontFace>(FrontFaceToNativeMap, p_frontFace);
}

VkPrimitiveTopology RenderTypeToNative::PrimitiveTopologyToNative(const PrimitiveTopology p_primitiveTopology)
{
   static const std::unordered_map<PrimitiveTopology, VkPrimitiveTopology> PrimitiveTopologyToNativeMap = {
       {PrimitiveTopology::PointList, VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
       {PrimitiveTopology::LineList, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
       {PrimitiveTopology::LineStrip, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP},
       {PrimitiveTopology::TriangleList, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
       {PrimitiveTopology::TriangleStrip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP},
       {PrimitiveTopology::TriangleFan, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN},
   };

   return Foundation::Util::EnumToNativeHelper<VkPrimitiveTopology>(PrimitiveTopologyToNativeMap, p_primitiveTopology);
}

VkPrimitiveTopology RenderTypeToNative::PrimitiveTopologyClassToNative(const PrimitiveTopologyClass p_primitiveTopologyClass)
{
   static const std::unordered_map<PrimitiveTopologyClass, VkPrimitiveTopology> PrimitiveTopologyClassToNativeMap = {
       {PrimitiveTopologyClass::Point, VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
       {PrimitiveTopologyClass::Line, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
       {PrimitiveTopologyClass::Triangle, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
   };

   return Foundation::Util::EnumToNativeHelper<VkPrimitiveTopology>(PrimitiveTopologyClassToNativeMap, p_primitiveTopologyClass);
}

VkCompareOp RenderTypeToNative::CompareOpToNative(const CompareOp p_compareOp)
{
   static const std::unordered_map<CompareOp, VkCompareOp> CompareOpToNativeMap = {
       {CompareOp::Never, VK_COMPARE_OP_NEVER},
       {CompareOp::Less, VK_COMPARE_OP_LESS},
       {CompareOp::Equal, VK_COMPARE_OP_EQUAL},
       {CompareOp::LessOrEqual, VK_COMPARE_OP_LESS_OR_EQUAL},
       {CompareOp::Greater, VK_COMPARE_OP_GREATER},
       {CompareOp::NotEqual, VK_COMPARE_OP_NOT_EQUAL},
       {CompareOp::GreaterOrEqual, VK_COMPARE_OP_GREATER_OR_EQUAL},
       {CompareOp::Always, VK_COMPARE_OP_ALWAYS},
   };

   return Foundation::Util::EnumToNativeHelper<VkCompareOp>(CompareOpToNativeMap, p_compareOp);
}

VkStencilFaceFlags RenderTypeToNative::StencilFaceFlagsToNative(const StencilFaceFlags p_stencilFaceFlags)
{
   {
      static const std::unordered_map<StencilFaceFlags, VkBufferUsageFlags> StencilFaceFlagsToNativeMap = {
          {StencilFaceFlags::Front, VK_STENCIL_FACE_FRONT_BIT},
          {StencilFaceFlags::Back, VK_STENCIL_FACE_BACK_BIT},
      };

      return Foundation::Util::FlagsToNativeHelper<VkStencilFaceFlags>(StencilFaceFlagsToNativeMap, p_stencilFaceFlags);
   }
}

VkStencilOp RenderTypeToNative::StencilOpToNative(const StencilOp p_stencilOp)
{
   static const std::unordered_map<StencilOp, VkStencilOp> StencilOpToNativeMap = {
       {StencilOp::Keep, VK_STENCIL_OP_KEEP},
       {StencilOp::Zero, VK_STENCIL_OP_ZERO},
       {StencilOp::Replace, VK_STENCIL_OP_REPLACE},
       {StencilOp::IntrecmentAndClamp, VK_STENCIL_OP_INCREMENT_AND_CLAMP},
       {StencilOp::DecrementAndClamp, VK_STENCIL_OP_DECREMENT_AND_CLAMP},
       {StencilOp::Invert, VK_STENCIL_OP_INVERT},
       {StencilOp::IncrementAndWrap, VK_STENCIL_OP_INCREMENT_AND_WRAP},
       {StencilOp::DecrementAndWrap, VK_STENCIL_OP_DECREMENT_AND_WRAP},
   };

   return Foundation::Util::EnumToNativeHelper<VkStencilOp>(StencilOpToNativeMap, p_stencilOp);
}

VkBlendFactor RenderTypeToNative::BlendFactorToNative(const BlendFactor p_blendFactor)
{
   static const std::unordered_map<BlendFactor, VkBlendFactor> BlendFactorToNativeMap = {
       {BlendFactor::FactorZero, VK_BLEND_FACTOR_ZERO},
       {BlendFactor::FactorOne, VK_BLEND_FACTOR_ONE},
       {BlendFactor::SrcColor, VK_BLEND_FACTOR_SRC_COLOR},
       {BlendFactor::OneMinusSrcColor, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR},
       {BlendFactor::DstColor, VK_BLEND_FACTOR_DST_COLOR},
       {BlendFactor::OneMinusDstColor, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR},
       {BlendFactor::SrcAlpha, VK_BLEND_FACTOR_SRC_ALPHA},
       {BlendFactor::OneMinusSrcAlpha, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
       {BlendFactor::DstAlpha, VK_BLEND_FACTOR_DST_ALPHA},
       {BlendFactor::OneMinusDstAlpha, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA},
       {BlendFactor::ConstantColor, VK_BLEND_FACTOR_CONSTANT_COLOR},
       {BlendFactor::OneMinusConstantColor, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR},
       {BlendFactor::ConstantAlpha, VK_BLEND_FACTOR_CONSTANT_ALPHA},
       {BlendFactor::OneMinusConstantAlpha, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA},
       {BlendFactor::SrcAlphaSaturate, VK_BLEND_FACTOR_SRC_ALPHA_SATURATE},
       {BlendFactor::Src1Color, VK_BLEND_FACTOR_SRC1_COLOR},
       {BlendFactor::OneMinusSrc1Color, VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR},
       {BlendFactor::Src1Alpha, VK_BLEND_FACTOR_SRC1_ALPHA},
       {BlendFactor::OneMinusSrc1Alpha, VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA},
   };

   return Foundation::Util::EnumToNativeHelper<VkBlendFactor>(BlendFactorToNativeMap, p_blendFactor);
}

VkBlendOp RenderTypeToNative::BlendOpToNative(const BlendOp p_blendOp)
{
   static const std::unordered_map<BlendOp, VkBlendOp> BlendFactorToNativeMap = {
       {BlendOp::Add, VK_BLEND_OP_ADD},
       {BlendOp::Subtract, VK_BLEND_OP_SUBTRACT},
       {BlendOp::ReverseSubtract, VK_BLEND_OP_REVERSE_SUBTRACT},
       {BlendOp::Min, VK_BLEND_OP_MIN},
       {BlendOp::Max, VK_BLEND_OP_MAX},
   };

   return Foundation::Util::EnumToNativeHelper<VkBlendOp>(BlendFactorToNativeMap, p_blendOp);
}

VkPipelineBindPoint RenderTypeToNative::PipelineBindPointToNative(const PipelineBindPoint p_pipelineBindPoint)
{
   static const std::unordered_map<PipelineBindPoint, VkPipelineBindPoint> PipelineBindPointToNativeMap = {
       {PipelineBindPoint::Graphics, VK_PIPELINE_BIND_POINT_GRAPHICS},
       {PipelineBindPoint::Compute, VK_PIPELINE_BIND_POINT_COMPUTE},
   };

   return Foundation::Util::EnumToNativeHelper<VkPipelineBindPoint>(PipelineBindPointToNativeMap, p_pipelineBindPoint);
}

VkIndexType RenderTypeToNative::IndexTypeToNative(const IndexType p_indexType)
{
   static const std::unordered_map<IndexType, VkIndexType> IndexTypeToNativeMap = {
       {IndexType::Uint16, VK_INDEX_TYPE_UINT16},
       {IndexType::Uint32, VK_INDEX_TYPE_UINT32},
   };

   return Foundation::Util::EnumToNativeHelper<VkIndexType>(IndexTypeToNativeMap, p_indexType);
}

VkColorComponentFlagBits RenderTypeToNative::ColorComponentFlagsToNative(const ColorComponentFlags p_colorComponentFlags)
{
   static const std::unordered_map<ColorComponentFlags, VkColorComponentFlagBits> ColorComponentFlagsToNativeMap = {
       {ColorComponentFlags::R, VK_COLOR_COMPONENT_R_BIT},
       {ColorComponentFlags::G, VK_COLOR_COMPONENT_G_BIT},
       {ColorComponentFlags::B, VK_COLOR_COMPONENT_B_BIT},
       {ColorComponentFlags::A, VK_COLOR_COMPONENT_A_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkColorComponentFlagBits>(ColorComponentFlagsToNativeMap, p_colorComponentFlags);
}

VkAttachmentLoadOp RenderTypeToNative::AttachmentLoadOpToNative(const AttachmentLoadOp p_attachmentLoadOp)
{
   static const std::unordered_map<AttachmentLoadOp, VkAttachmentLoadOp> AttachmentLoadOpToNativeMap = {
       {AttachmentLoadOp::Load, VK_ATTACHMENT_LOAD_OP_LOAD},
       {AttachmentLoadOp::Clear, VK_ATTACHMENT_LOAD_OP_CLEAR},
       {AttachmentLoadOp::DontCare, VK_ATTACHMENT_LOAD_OP_DONT_CARE},
   };

   return Foundation::Util::FlagsToNativeHelper<VkAttachmentLoadOp>(AttachmentLoadOpToNativeMap, p_attachmentLoadOp);
}

VkAttachmentStoreOp RenderTypeToNative::AttachmentStoreOpToNative(const AttachmentStoreOp p_attachmentStoreOp)
{
   static const std::unordered_map<AttachmentStoreOp, VkAttachmentStoreOp> AttachmentStoreOpToNativeMap = {
       {AttachmentStoreOp::Store, VK_ATTACHMENT_STORE_OP_STORE},
       {AttachmentStoreOp::DontCare, VK_ATTACHMENT_STORE_OP_DONT_CARE},
   };

   return Foundation::Util::FlagsToNativeHelper<VkAttachmentStoreOp>(AttachmentStoreOpToNativeMap, p_attachmentStoreOp);
}

VkFormat RenderTypeToNative::ResourceFormatToNative(ResourceFormat p_format)
{
   static const std::unordered_map<ResourceFormat, VkFormat> ResourceFormatToNativeMap = {
       {ResourceFormat::Undefined, VK_FORMAT_UNDEFINED},

       {ResourceFormat::R4G4UnormPack8, VK_FORMAT_R4G4_UNORM_PACK8},
       {ResourceFormat::R4G4B4A4UnormPack16, VK_FORMAT_R4G4B4A4_UNORM_PACK16},
       {ResourceFormat::B4G4R4A4UnormPack16, VK_FORMAT_B4G4R4A4_UNORM_PACK16},
       {ResourceFormat::R5G6B5UnormPack16, VK_FORMAT_R5G6B5_UNORM_PACK16},
       {ResourceFormat::B5G6R5UnormPack16, VK_FORMAT_B5G6R5_UNORM_PACK16},
       {ResourceFormat::R5G5B5A1UnormPack16, VK_FORMAT_R5G5B5A1_UNORM_PACK16},
       {ResourceFormat::B5G5R5A1UnormPack16, VK_FORMAT_B5G5R5A1_UNORM_PACK16},
       {ResourceFormat::A1R5G5B5UnormPack16, VK_FORMAT_A1R5G5B5_UNORM_PACK16},

       {ResourceFormat::R8Unorm, VK_FORMAT_R8_UNORM},
       {ResourceFormat::R8Snorm, VK_FORMAT_R8_SNORM},
       {ResourceFormat::R8Scaled, VK_FORMAT_R8_USCALED},
       {ResourceFormat::R8SScaled, VK_FORMAT_R8_SSCALED},
       {ResourceFormat::R8Uint, VK_FORMAT_R8_UINT},
       {ResourceFormat::R8Sint, VK_FORMAT_R8_SINT},
       {ResourceFormat::R8Srgb, VK_FORMAT_R8_SRGB},

       {ResourceFormat::R8G8Unorm, VK_FORMAT_R8G8_UNORM},
       {ResourceFormat::R8G8Snorm, VK_FORMAT_R8G8_SNORM},
       {ResourceFormat::R8G8Uscaled, VK_FORMAT_R8G8_USCALED},
       {ResourceFormat::R8G8Sscaled, VK_FORMAT_R8G8_SSCALED},
       {ResourceFormat::R8G8Uint, VK_FORMAT_R8G8_UINT},

       {ResourceFormat::R8G8B8A8Unorm, VK_FORMAT_R8G8B8A8_UNORM},
       {ResourceFormat::B8G8R8A8Srgb, VK_FORMAT_B8G8R8A8_SRGB},
       {ResourceFormat::R32G32Sfloat, VK_FORMAT_R32G32_SFLOAT},
       {ResourceFormat::R32G32B32Sfloat, VK_FORMAT_R32G32B32_SFLOAT},

       {ResourceFormat::D16Unorm, VK_FORMAT_D16_UNORM},
       {ResourceFormat::X8D24UnormPack32, VK_FORMAT_X8_D24_UNORM_PACK32},
       {ResourceFormat::D32Sfloat, VK_FORMAT_D32_SFLOAT},
       {ResourceFormat::S8Uint, VK_FORMAT_S8_UINT},
       {ResourceFormat::D16UnormS8Uint, VK_FORMAT_D16_UNORM_S8_UINT},
       {ResourceFormat::D24UnormS8Uint, VK_FORMAT_D24_UNORM_S8_UINT},
       {ResourceFormat::D32SfloatS8Uint, VK_FORMAT_D32_SFLOAT_S8_UINT},

       {ResourceFormat::Invalid, VK_FORMAT_UNDEFINED}};

   return Foundation::Util::EnumToNativeHelper<VkFormat>(ResourceFormatToNativeMap, p_format);
}

VkImageViewType RenderTypeToNative::ImageViewTypeToNative(ImageViewType p_viewType)
{
   static const std::unordered_map<ImageViewType, VkImageViewType> ImageViewTypeToNativeMap = {
       {ImageViewType::View1D, VK_IMAGE_VIEW_TYPE_1D},
       {ImageViewType::View2D, VK_IMAGE_VIEW_TYPE_2D},
       {ImageViewType::View3D, VK_IMAGE_VIEW_TYPE_3D},
       {ImageViewType::ViewCube, VK_IMAGE_VIEW_TYPE_CUBE},
       {ImageViewType::View1DArray, VK_IMAGE_VIEW_TYPE_1D_ARRAY},
       {ImageViewType::View2DArray, VK_IMAGE_VIEW_TYPE_2D_ARRAY},
       {ImageViewType::ViewCubeArray, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY},

       {ImageViewType::Invalid, VK_IMAGE_VIEW_TYPE_MAX_ENUM}};

   return Foundation::Util::EnumToNativeHelper<VkImageViewType>(ImageViewTypeToNativeMap, p_viewType);
}

VkImageAspectFlagBits RenderTypeToNative::ImageAspectFlagsToNative(const ImageAspectFlags p_aspectFlags)
{
   static const std::unordered_map<ImageAspectFlags, VkImageAspectFlagBits> ImageAspectFlagsToNativeMap = {
       {ImageAspectFlags::Color, VK_IMAGE_ASPECT_COLOR_BIT},     {ImageAspectFlags::Depth, VK_IMAGE_ASPECT_DEPTH_BIT},
       {ImageAspectFlags::Stencil, VK_IMAGE_ASPECT_STENCIL_BIT}, {ImageAspectFlags::MetaData, VK_IMAGE_ASPECT_METADATA_BIT},
       {ImageAspectFlags::Plane0, VK_IMAGE_ASPECT_PLANE_0_BIT},  {ImageAspectFlags::Plane1, VK_IMAGE_ASPECT_PLANE_1_BIT},
       {ImageAspectFlags::Plane2, VK_IMAGE_ASPECT_PLANE_2_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkImageAspectFlagBits>(ImageAspectFlagsToNativeMap, p_aspectFlags);
}

VkVertexInputRate RenderTypeToNative::VertexInputRateToNative(const VertexInputRate p_vertexInputRate)
{
   static const std::unordered_map<VertexInputRate, VkVertexInputRate> ImageCreationFlagsToNativeMap = {
       {VertexInputRate::VertexInputRateVertex, VK_VERTEX_INPUT_RATE_VERTEX},
       {VertexInputRate::VertexInputRateInstance, VK_VERTEX_INPUT_RATE_INSTANCE},
   };

   return Foundation::Util::EnumToNativeHelper<VkVertexInputRate>(ImageCreationFlagsToNativeMap, p_vertexInputRate);
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
