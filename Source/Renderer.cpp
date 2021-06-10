#include <Renderer.h>

#include <glad/vulkan.h>

#include <EASTL/sort.h>

#include <Util/MurmurHash3.h>

#include <DescriptorSetLayout.h>

namespace Render
{
uint64_t RendererHelper::CalculateHashFromDescriptorSetLayoutDescriptor(DescriptorSetLayoutDescriptor& p_desc)
{
   // Sort the DescriptorSetLayoutBindings in order of it
   const auto predicate = [](const VkDescriptorSetLayoutBinding& p_a, const VkDescriptorSetLayoutBinding& p_b) {
      return static_cast<uint32_t>(p_a.descriptorType) < static_cast<uint32_t>(p_b.descriptorType);
   };
   eastl::sort(p_desc.m_layoutBindings.begin(), p_desc.m_layoutBindings.end(), predicate);

   // Hash the array
   constexpr uint32_t seed = 1991u;

   uint64_t generatedHash = 0u;
   MurmurHash3_x64_64(p_desc.m_layoutBindings.data(),
                      static_cast<int>(p_desc.m_layoutBindings.size()) * sizeof(VkDescriptorSetLayoutBinding), seed,
                      (void*)&generatedHash);

   return generatedHash;
}
} // namespace Render
