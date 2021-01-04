#include <EASTL/sort.h>

#include <Util/MurmurHash3.h>

#include <DescriptorSetLayoutManager.h>
#include <DescriptorSetLayout.h>

#include <std/vector.h>

namespace Render
{

DescriptorSetLayoutManager::~DescriptorSetLayoutManager()
{
}

eastl::weak_ptr<DescriptorSetLayout*>
DescriptorSetLayoutManager::CreateOrGetDescriptorSetLayout(DescriptorSetlayoutDescriptor&& p_desc)
{
   std::lock_guard<std::mutex> lock(m_descriptorSetLayoutMapMutex);
   // Check if a new DescriptorSetLayout needs to be created, or can be re-used
   auto descriptorSetLayoutItr = m_descriptorSetLayoutMap.find(p_desc.GetHash());
   // If there is none yet, create it
   if (descriptorSetLayoutItr == m_descriptorSetLayoutMap.end())
   {
      eastl::unique_ptr<DescriptorSetLayout>& descriptorSetLayout = m_descriptorSetLayoutMap[p_desc.GetHash()];
      descriptorSetLayout = DescriptorSetLayout::CreateInstance(eastl::move(p_desc));
      return descriptorSetLayout.get()->GetDescriptorSetLayoutReference();
   }

   // Return the existing pointer
   return descriptorSetLayoutItr->second.get()->GetDescriptorSetLayoutReference();
}

}; // namespace Render
