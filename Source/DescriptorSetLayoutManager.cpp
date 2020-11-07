#include <EASTL/sort.h>

#include <Util/MurmurHash3.h>

#include <DescriptorSetLayoutManager.h>
#include <DescriptorSetLayout.h>

#include <std/vector.h>

namespace Render
{

DescriptorSetLayoutManager::DescriptorSetLayoutManager()
{
}

DescriptorSetLayoutManager::~DescriptorSetLayoutManager()
{
}

DescriptorSetLayout* DescriptorSetLayoutManager::CreateOrGetDescriptorSetLayout(DescriptorSetlayoutDescriptor&& p_desc)
{
   // Sort the DescriptorSetLayoutBindings in order of it
   const auto predicate = [](VkDescriptorSetLayoutBinding* p_a, VkDescriptorSetLayoutBinding* p_b) {
      return static_cast<uint32_t>(p_a->descriptorType) < static_cast<uint32_t>(p_b->descriptorType);
   };
   eastl::sort(p_desc.m_layoutBindings.begin(), p_desc.m_layoutBindings.end(), predicate);

   // Hash the array
   constexpr uint32_t seed = 1991u;
   uint64_t descriptorSetLayoutBindingHash = 0u;
   MurmurHash3_x64_64(p_desc.m_layoutBindings.data(), p_desc.m_layoutBindings.size() * sizeof(VkDescriptorSetLayoutBinding), seed,
                      (void*)&descriptorSetLayoutBindingHash);

   std::lock_guard<std::mutex> lock(m_descriptorSetLayoutMapMutex);
   // Check if a new DescriptorSetLayout needs to be created, or can be re-used
   auto descriptorSetLayoutItr = m_descriptorSetLayoutMap.find(descriptorSetLayoutBindingHash);
   // If there is none yet, create it
   if (descriptorSetLayoutItr == m_descriptorSetLayoutMap.end())
   {
      eastl::unique_ptr<DescriptorSetLayout>& descriptorSetLayout = m_descriptorSetLayoutMap[descriptorSetLayoutBindingHash];
      descriptorSetLayout = DescriptorSetLayout::CreateInstance(eastl::move(p_desc));
      return descriptorSetLayout.get();
   }

   // Return the existing pointer
   return descriptorSetLayoutItr->second.get();
}

}; // namespace Render
