#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>

class DescriptorSet
{
 public:
   static constexpr size_t MaxDescriptorSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Shader, 12u, static_cast<uint32_t>(sizeof(Shader) * MaxDescriptorSetCountPerPage));

   struct Descriptor
   {
   };

 private:
};
