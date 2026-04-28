#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <memory>

#include <glm/glm.hpp>

#include <GHI/Renderer.h>

namespace Render
{

namespace GHI
{
#include <type_traits>
#include <cstdint>

template <class E>
struct enable_bitmask_operators : std::false_type
{
};

// Helper: C++20 to_underlying (use std::to_underlying if you’re on C++23)
template <class E>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
{
   return static_cast<std::underlying_type_t<E>>(e);
}

// Constrain all ops to enums that opted in
template <class E>
concept BitmaskEnum = std::is_enum_v<E> && enable_bitmask_operators<E>::value;

// ~, |, &, ^ and their assignment versions
template <BitmaskEnum E>
constexpr E operator~(E v) noexcept
{
   using U = std::underlying_type_t<E>;
   return static_cast<E>(~static_cast<U>(v));
}
template <BitmaskEnum E>
constexpr E operator|(E a, E b) noexcept
{
   using U = std::underlying_type_t<E>;
   return static_cast<E>(static_cast<U>(a) | static_cast<U>(b));
}
template <BitmaskEnum E>
constexpr E operator&(E a, E b) noexcept
{
   using U = std::underlying_type_t<E>;
   return static_cast<E>(static_cast<U>(a) & static_cast<U>(b));
}
template <BitmaskEnum E>
constexpr E operator^(E a, E b) noexcept
{
   using U = std::underlying_type_t<E>;
   return static_cast<E>(static_cast<U>(a) ^ static_cast<U>(b));
}
template <BitmaskEnum E>
constexpr E& operator|=(E& a, E b) noexcept
{
   return a = (a | b);
}
template <BitmaskEnum E>
constexpr E& operator&=(E& a, E b) noexcept
{
   return a = (a & b);
}
template <BitmaskEnum E>
constexpr E& operator^=(E& a, E b) noexcept
{
   return a = (a ^ b);
}

// Convenience checks
template <BitmaskEnum E>
constexpr bool any(E value, E test) noexcept
{
   return (to_underlying(value) & to_underlying(test)) != 0;
}
template <BitmaskEnum E>
constexpr bool all(E value, E test) noexcept
{
   return (to_underlying(value) & to_underlying(test)) == to_underlying(test);
}

template <typename T>
using Ptr = std::shared_ptr<T>;

template <typename T>
using ConstPtr = std::shared_ptr<const T>;

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <typename T>
using ConstWeakPtr = std::weak_ptr<const T>;

// Debug = checked (dynamic), Release = fast (static).
template <class To, class From>
std::shared_ptr<To> Cast(const std::shared_ptr<From>& p)
{
#if !defined(NDEBUG)
   auto q = std::dynamic_pointer_cast<To>(p);
   assert(q && "ptr_cast failed: wrong concrete type");
   return q;
#else
   return std::static_pointer_cast<To>(p);
#endif
}

template <class To, class From>
std::shared_ptr<const To> Cast(const std::shared_ptr<const From>& p)
{
#if !defined(NDEBUG)
   auto q = std::dynamic_pointer_cast<const To>(p);
   assert(q && "ptr_cast failed: wrong concrete type");
   return q;
#else
   return std::static_pointer_cast<const To>(p);
#endif
}

static constexpr uint64_t WholeSize = static_cast<uint64_t>(-1); // TODO: VK_WHOLE_SIZE;

struct Rect2D
{
   glm::ivec2 m_offset;
   glm::uvec2 m_extent;
};

struct ViewportRect
{
   glm::vec2 m_position;
   glm::vec2 m_size;
   float m_minDepth;
   float m_maxDepth;
};

struct BufferCopyDescriptor
{
   uint64_t m_srcOffset;
   uint64_t m_dstOffset;
   uint64_t m_size;
};

enum class ResourceFormat : uint32_t
{
   Undefined = 0u,
   R4G4UnormPack8,
   R4G4B4A4UnormPack16,
   B4G4R4A4UnormPack16,
   R5G6B5UnormPack16,
   B5G6R5UnormPack16,
   R5G5B5A1UnormPack16,
   B5G5R5A1UnormPack16,
   A1R5G5B5UnormPack16,
   R8Unorm,
   R8Snorm,
   R8Scaled,
   R8SScaled,
   R8Uint,
   R8Sint,
   R8Srgb,
   R8G8Unorm,
   R8G8Snorm,
   R8G8Uscaled,
   R8G8Sscaled,
   R8G8Uint,
   // TODO: more (formats 21–49 not yet enumerated)

   B8G8R8A8Srgb = 50u,      // VK_FORMAT_B8G8R8A8_SRGB
   // TODO: more (formats 51–105 not yet enumerated)

   R32G32B32Sfloat = 106u,  // VK_FORMAT_R32G32B32_SFLOAT
   // TODO: more (formats 107-123 not yet enumerated)

   D16Unorm = 124u,          // VK_FORMAT_D16_UNORM
   X8D24UnormPack32,         // VK_FORMAT_X8_D24_UNORM_PACK32
   D32Sfloat,                // VK_FORMAT_D32_SFLOAT
   S8Uint,                   // VK_FORMAT_S8_UINT
   D16UnormS8Uint,           // VK_FORMAT_D16_UNORM_S8_UINT
   D24UnormS8Uint,           // VK_FORMAT_D24_UNORM_S8_UINT
   D32SfloatS8Uint,          // VK_FORMAT_D32_SFLOAT_S8_UINT

   Count,
   Invalid = Count
};

enum class ImageLayout : uint32_t
{
   Undefined = 0u,
   General,
   ColorAttachment,
   DepthStencilAttachment,
   DepthStencilReadOnly,
   ShaderRead,
   TransferSrc,
   TransferDst,
   DepthReadOnlyStencilAttachment,
   // TODO: more

   Count,
   Invalid = Count
};

enum class ImageType : uint32_t
{
   Image1D = 0u,
   Image2D,
   Image3D,

   Count,
   Invalid = Count
};

enum class ImageViewType : uint32_t
{
   View1D,
   View2D,
   View3D,
   ViewCube,
   View1DArray,
   View2DArray,
   ViewCubeArray,

   Count,
   Invalid = Count
};

enum class VertexInputRate : uint32_t
{
   VertexInputRateVertex = 0u,
   VertexInputRateInstance,

   Count,
   Invalid = Count
};

enum class ImageAspectFlags : uint32_t
{
   Color = (1 << 0),
   Depth = (1 << 1),
   Stencil = (1 << 2),
   MetaData = (1 << 3),
   Plane0 = (1 << 4),
   Plane1 = (1 << 5),
   Plane2 = (1 << 6),
};

template <>
struct enable_bitmask_operators<ImageAspectFlags> : std::true_type
{
};

enum class ImageCreationFlags : uint32_t
{
   Alias = (1 << 0),
   Cube_Or_CubeArray = (1 << 1),
   Array2D = (1 << 2),
   // TODO: Add sparse image support
};

enum class ImageUsageFlags : uint32_t
{
   TransferSource = (1 << 0),
   TransferDestination = (1 << 1),
   Sampled = (1 << 2),
   Storage = (1 << 3),
   ColorAttachment = (1 << 4),
   DepthStencilAttachment = (1 << 5),
   TransientAttachment = (1 << 6),
   InputAttachment = (1 << 7),
};

enum class ImageTiling : uint32_t
{
   TilingOptimal = 0u,
   TilingLinear,

   Count,
   Invalid = Count
};

enum class DescriptorPoolType : uint32_t
{
   Resource = 0u,
   Sampler,

   Count,
   Invalid = Count
};

enum class QueueTypeFlags : uint32_t
{
   GraphicsQueue = (1 << 0),
   ComputeQueue = (1 << 1),
   TransferQueue = (1 << 2),

   AllQueues = GraphicsQueue + ComputeQueue + TransferQueue,
};

template <>
struct enable_bitmask_operators<QueueTypeFlags> : std::true_type
{
};

enum class PhysicalDeviceFeatureFlags : uint32_t
{
   Presenting = (1 << 0),
   Swapchain = (1 << 1),
};

template <>
struct enable_bitmask_operators<PhysicalDeviceFeatureFlags> : std::true_type
{
};

enum class GPUType : uint32_t
{
   Discrete = 0u,
   Integrated = 1u,

   Count,
   Invalid = Count,
};

template <>
struct enable_bitmask_operators<GPUType> : std::true_type
{
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

template <>
struct enable_bitmask_operators<MemoryPropertyFlags> : std::true_type
{
};

enum class PipelineStageFlags : uint32_t
{
   None = 0,

   TopOfPipe = (1 << 0),
   DrawIndirect = (1 << 1),
   VertexInput = (1 << 2),
   VertexShader = (1 << 3),
   TessControlShader = (1 << 4),
   TessEvalShader = (1 << 5),
   GeometryShader = (1 << 6),
   FragmentShader = (1 << 7),
   EarlyFragmentTests = (1 << 8),
   LateFragmentTests = (1 << 9),
   ColorAttachmentOut = (1 << 10),
   ComputeShader = (1 << 11),
   Transfer = (1 << 12),
   BottomOfPipe = (1 << 13),
   Host = (1 << 14),
   AllGraphics = (1 << 15),
   AllCommands = (1 << 16),
};

template <>
struct enable_bitmask_operators<PipelineStageFlags> : std::true_type
{
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

template <>
struct enable_bitmask_operators<ShaderStageFlag> : std::true_type
{
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

template <>
struct enable_bitmask_operators<BufferUsageFlags> : std::true_type
{
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

template <>
struct enable_bitmask_operators<StencilFaceFlags> : std::true_type
{
};

enum class FrameBufferCreateFlags : uint32_t
{
   CreateImageless = (1u << 0),
};

template <>
struct enable_bitmask_operators<FrameBufferCreateFlags> : std::true_type
{
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

template <>
struct enable_bitmask_operators<ColorComponentFlags> : std::true_type
{
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
   Load = 0u,
   Clear,
   DontCare,

   Count,
   Invalid = Count
};

enum class AttachmentStoreOp : uint32_t
{
   Store = 0u,
   DontCare,

   Count,
   Invalid = Count
};

enum class AccessFlags : uint32_t
{
   None = 0,

   IndirectCommandRead = (1 << 0),
   IndexRead = (1 << 1),
   VertexAttributeRead = (1 << 2),
   UniformRead = (1 << 3),
   InputAttachmentRead = (1 << 4),
   ShaderRead = (1 << 5),
   ShaderWrite = (1 << 6),
   ColorAttachmentRead = (1 << 7),
   ColorAttachmentWrite = (1 << 8),
   DepthStencilAttachmentRead = (1 << 9),
   DepthStencilAttachmentWrite = (1 << 10),
   TransferRead = (1 << 11),
   TransferWrite = (1 << 12),
   HostRead = (1 << 13),
   HostWrite = (1 << 14),
   MemoryRead = (1 << 15),
   MemoryWrite = (1 << 16),
};

template <>
struct enable_bitmask_operators<AccessFlags> : std::true_type
{
};

enum class ResolveModeFlags : uint32_t
{
   None = 0,

   SampleZero = (1 << 0), // Take sample 0
   Average = (1 << 1),    // Average of samples
   Min = (1 << 2),        // Minimum value of samples
   Max = (1 << 3),        // Maximum value of samples
};

template <>
struct enable_bitmask_operators<ResolveModeFlags> : std::true_type
{
};

union ClearColorValue
{
   glm::vec4 m_clearValFloat;
   glm::ivec4 m_clearValInt;
   glm::uvec4 m_clearValUInt;
};

}; // namespace GHI

}; // namespace Render
