#include <GHI/Vulkan/Buffer.h>

#include <Util/Assert.h>

#include <GHI/AsyncUploadQueueInterface.h>

#include <GHI/Vulkan/RendererTypes.h>
#include <GHI/Vulkan/Fence.h>
#include <GHI/Vulkan/AsyncUploadQueue.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

namespace Internal
{

VkDevice NativeDevice(Ptr<GHI::Device> p_device)
{
   auto vulkanDevice = GHI::Cast<GHI::Vulkan::Device>(p_device);
   return vulkanDevice->GetLogicalDeviceNative();
}

} // namespace Internal

} // namespace

Buffer::Buffer(Ptr<GHI::Device> p_device, BufferDescriptor&& p_desc) : GHI::Buffer(p_device, std::move(p_desc))
{
   VkBufferCreateInfo bufferCreateInfo = {};
   bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferCreateInfo.pNext = nullptr;
   bufferCreateInfo.flags = 0u;
   bufferCreateInfo.size = GetDesc().m_requestBufferSize;
   bufferCreateInfo.usage = RenderTypeToNative::BufferUsageFlagsToNative(GetDesc().m_bufferUsageFlags);
   bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   bufferCreateInfo.queueFamilyIndexCount = 0u;
   bufferCreateInfo.pQueueFamilyIndices = nullptr;

   VkResult res = vkCreateBuffer(Internal::NativeDevice(m_device), &bufferCreateInfo, nullptr, &m_bufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a Buffer resource");

   // Create the memory
   VkMemoryRequirements memoryRequirements;
   vkGetBufferMemoryRequirements(Internal::NativeDevice(m_device), m_bufferNative, &memoryRequirements);
   auto [deviceMemory, allocatedMemory] =
       GHI::Cast<GHI::Vulkan::Device>(m_device)->AllocateDeviceMemory(memoryRequirements, GetDesc().m_memoryProperties);
   m_deviceMemory = deviceMemory;
   m_bufferSizeAllocatedMemory = allocatedMemory;

   // Bind the Buffer resource to the Memory resource
   res = vkBindBufferMemory(Internal::NativeDevice(m_device), GetBufferNative(), GetDeviceMemoryNative(), 0u);
   ASSERT(res == VK_SUCCESS, "Failed to bind the Buffer resource to the Memory resource");
}

Buffer::~Buffer()
{
   ASSERT(m_deviceMemory != VK_NULL_HANDLE, "Memory not valid. Trying to cleanup a buffer that was never initialized");
   vkFreeMemory(Internal::NativeDevice(m_device), m_deviceMemory, nullptr);

   ASSERT(m_bufferNative != VK_NULL_HANDLE, "Buffer not valid. Trying to cleanup a buffer that was never initialized");
   vkDestroyBuffer(Internal::NativeDevice(m_device), m_bufferNative, nullptr);
}

const VkBuffer Buffer::GetBufferNative() const
{
   return m_bufferNative;
}

const VkDeviceMemory Buffer::GetDeviceMemoryNative() const
{
   return m_deviceMemory;
}

const BufferUsageFlags Buffer::GetUsageFlags() const
{
   return GetDesc().m_bufferUsageFlags;
}

const uint64_t Buffer::GetBufferSizeRequested() const
{
   return GetDesc().m_requestBufferSize;
}

const uint64_t Buffer::GetBufferSizeAllocated() const
{
   return m_bufferSizeAllocatedMemory;
}

Ptr<GHI::Fence> Buffer::UploadDataInternal(const void* p_data, uint64_t p_dataSize)
{
   GHI::BufferUploadRequest uploadRequest{.m_sourceData = p_data,
                                          .m_copySizeInBytes = p_dataSize,
                                          .m_destBuffer = GHI::Cast<GHI::Buffer>(shared_from_this()),
                                          .m_destOffsetInBytes = 0u};

   std::vector<GHI::BufferUploadRequest> uploadRequests{uploadRequest};
   Ptr<GHI::Fence> fence = AsyncUploadQueueInterface::Get()->QueueUpload(uploadRequests);

   return fence;
}

void Buffer::UploadDataImmediateInternal(const void* p_data, uint64_t p_dataSize)
{
   Ptr<GHI::Fence> uploadFence = UploadDataInternal(p_data, p_dataSize);

   m_device->WaitFences({FenceSubmitInfo{.m_fence = uploadFence, .m_value = 1u}});
}

void* Buffer::MapInternal(uint64_t p_offset, uint64_t p_size /*= WholeSize*/)
{
   ASSERT(p_size == WholeSize && p_offset < GetDesc().m_requestBufferSize, "Mapped data range out of bounds");
   // TODO: Should we support multiple mapped regions?
   ASSERT(m_mappedData == nullptr, "Buffer is already mapped");

   vkMapMemory(Internal::NativeDevice(m_device), GetDeviceMemoryNative(), p_offset, GetBufferSizeAllocated(), {}, &m_mappedData);

   return m_mappedData;
}

void Buffer::UnmapInternal()
{
   ASSERT(m_mappedData != nullptr, "Buffer isn't mapped");

   vkUnmapMemory(Internal::NativeDevice(m_device), GetDeviceMemoryNative());
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
