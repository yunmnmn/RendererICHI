#include <GHI/DescriptorSetWriter.h>

#include <Util/Assert.h>

#include <GHI/DescriptorSet.h>

namespace Render
{

namespace GHI
{

DescriptorSetWriter::DescriptorSetWriter(DescriptorSet* p_set)
    : m_set(p_set)
{
}

DescriptorSetWriter& DescriptorSetWriter::WriteUniformBuffer(std::string_view p_bindingName, Ptr<BufferView> p_bufferView)
{
   m_pendingWrites.push_back({std::string(p_bindingName), PendingWrite::WriteType::UniformBuffer, p_bufferView, nullptr});
   return *this;
}

DescriptorSetWriter& DescriptorSetWriter::WriteStorageBuffer(std::string_view p_bindingName, Ptr<BufferView> p_bufferView)
{
   m_pendingWrites.push_back({std::string(p_bindingName), PendingWrite::WriteType::StorageBuffer, p_bufferView, nullptr});
   return *this;
}

DescriptorSetWriter& DescriptorSetWriter::WriteSampledImage(std::string_view p_bindingName, Ptr<ImageView> p_imageView)
{
   m_pendingWrites.push_back({std::string(p_bindingName), PendingWrite::WriteType::SampledImage, nullptr, p_imageView});
   return *this;
}

DescriptorSetWriter& DescriptorSetWriter::WriteStorageImage(std::string_view p_bindingName, Ptr<ImageView> p_imageView)
{
   m_pendingWrites.push_back({std::string(p_bindingName), PendingWrite::WriteType::StorageImage, nullptr, p_imageView});
   return *this;
}

void DescriptorSetWriter::Compile()
{
   ASSERT(m_set != nullptr, "DescriptorSetWriter::Compile called on an already-consumed writer");
   m_set->CompileWrites(std::move(m_pendingWrites));
   m_set = nullptr;
}

} // namespace GHI

} // namespace Render
