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
   auto vulkanDevice =  GHI::Cast<GHI::Vulkan::Device>(p_device);
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
   bufferCreateInfo.size = p_desc.m_requestBufferSize;
   bufferCreateInfo.usage = RenderTypeToNative::BufferUsageFlagsToNative(p_desc.m_bufferUsageFlags);
   bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   bufferCreateInfo.queueFamilyIndexCount = 0u;
   bufferCreateInfo.pQueueFamilyIndices = nullptr;

   VkResult res = vkCreateBuffer(Internal::NativeDevice(m_device), &bufferCreateInfo, nullptr, &m_bufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a Buffer resource");

   // Create the memory
   VkMemoryRequirements memoryRequirements;
   vkGetBufferMemoryRequirements(Internal::NativeDevice(m_device), m_bufferNative, &memoryRequirements);
   auto [deviceMemory, allocatedMemory] = GHI::Cast<GHI::Vulkan::Device>(m_device)->AllocateDeviceMemory(
       memoryRequirements, GetDesc().m_memoryProperties);
   m_deviceMemory = deviceMemory;
   m_bufferSizeAllocatedMemory = allocatedMemory;

   // Bind the Buffer resource to the Memory resource
   res = vkBindBufferMemory(Internal::NativeDevice(m_device), GetBufferNative(), GetDeviceMemoryNative(), 0u);
   ASSERT(res == VK_SUCCESS, "Failed to bind the Buffer resource to the Memory resource");

   if (p_desc.m_initialData)
   {
      BufferUploadRequest uploadRequest{.m_sourceData = p_desc.m_initialData,
                                        .m_copySizeInBytes = p_desc.m_initialDataSize,
                                        .m_destBuffer = this,
                                        .m_destOffsetInBytes = 0u};

      std::vector<BufferUploadRequest> uploadRequests{uploadRequest};
      Ptr<Fence> fence = AsyncUploadQueueInterface::Get()->QueueUpload(uploadRequests);
      fence->WaitForSignal();
   }
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

void* Buffer::MapInternal(uint64_t p_offset, uint64_t p_size /*= WholeSize*/)
{
   ASSERT(p_size == WholeSize && p_size + p_offset < GetDesc().m_requestBufferSize, "Mapped data range out of bounds");
   // TODO: Should we support multiple mapped regions?
   ASSERT(m_mappedData == nullptr, "Buffer is already mapped");

   vkMapMemory(Internal::NativeDevice(m_device), GetDeviceMemoryNative(), p_offset, GetBufferSizeAllocated(), {},
               &m_mappedData);

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
