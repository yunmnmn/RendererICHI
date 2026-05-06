#pragma once

#include <string>
#include <vector>

#include <GHI/BufferView.h>
#include <GHI/ImageView.h>
#include <GHI/Sampler.h>

namespace Render
{

namespace GHI
{

class DescriptorSet;

struct PendingWrite
{
   enum class WriteType
   {
      UniformBuffer,
      StorageBuffer,
      SampledImage,
      StorageImage,
      Sampler,
   };

   std::string m_bindingName;
   WriteType m_type = WriteType::UniformBuffer;
   Ptr<BufferView> m_bufferView;
   Ptr<ImageView> m_imageView;
   Ptr<Sampler> m_sampler;
};

// Move-only writer returned by DescriptorSet::BeginWrite().
// Accumulates resource view writes and flushes them to the descriptor pool on Compile().
class DescriptorSetWriter
{
   friend class DescriptorSet;

 public:
   DescriptorSetWriter() = delete;
   DescriptorSetWriter(const DescriptorSetWriter&) = delete;
   DescriptorSetWriter& operator=(const DescriptorSetWriter&) = delete;
   DescriptorSetWriter(DescriptorSetWriter&&) = default;
   DescriptorSetWriter& operator=(DescriptorSetWriter&&) = default;

   DescriptorSetWriter& WriteUniformBuffer(std::string_view p_bindingName, Ptr<BufferView> p_bufferView);
   DescriptorSetWriter& WriteStorageBuffer(std::string_view p_bindingName, Ptr<BufferView> p_bufferView);
   DescriptorSetWriter& WriteSampledImage(std::string_view p_bindingName, Ptr<ImageView> p_imageView);
   DescriptorSetWriter& WriteStorageImage(std::string_view p_bindingName, Ptr<ImageView> p_imageView);
   DescriptorSetWriter& WriteSampler(std::string_view p_bindingName, Ptr<Sampler> p_sampler);

   // Flush all pending writes to the descriptor pool. QueueSubmit tracks in-flight
   // versions automatically, so no fence parameters are needed here.
   void Compile();

 private:
   explicit DescriptorSetWriter(DescriptorSet* p_set);

   DescriptorSet* m_set = nullptr;
   std::vector<PendingWrite> m_pendingWrites;
};

} // namespace GHI

} // namespace Render
