#include <DescriptorSetLayoutManager.h>

#include <std/vector.h>
#include <EASTL/sort.h>

#include <Util/MurmurHash3.h>

#include <DescriptorSetLayout.h>

namespace Render
{

namespace
{
namespace Internal
{
uint64_t CalculateHashFromDescriptorSetLayoutDescriptor(const DescriptorSetLayoutDescriptor& p_desc)
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

} // namespace Internal
} // namespace

DescriptorSetLayoutManager::DescriptorSetLayoutManager(DescriptorSetLayoutManagerDescriptor&& p_desc)
{
   m_vulkanDevice = p_desc.m_vulkanDevice;
}

DescriptorSetLayoutManager::~DescriptorSetLayoutManager()
{
}

ResourceRef<DescriptorSetLayout> DescriptorSetLayoutManager::CreateOrGetDescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc)
{
   std::lock_guard<std::mutex> lock(m_descriptorSetLayoutMapMutex);
   const uint64_t generatedHash = Internal::CalculateHashFromDescriptorSetLayoutDescriptor(p_desc);
   // Check if a new DescriptorSetLayout needs to be created, or can be re-used
   auto descriptorSetLayoutItr = m_descriptorSetLayoutMap.find(generatedHash);
   // If there is none yet, create it
   if (descriptorSetLayoutItr == m_descriptorSetLayoutMap.end())
   {
      ResourceRef<DescriptorSetLayout>& descriptorSetLayout = m_descriptorSetLayoutMap[generatedHash];
      descriptorSetLayout = DescriptorSetLayout::CreateInstance(eastl::move(p_desc));
      return descriptorSetLayout;
   }

   // Return the existing pointer
   return descriptorSetLayoutItr->second;
}

}; // namespace Render
