#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>

#include <Util/HashName.h>
#include <Memory/ClassAllocator.h>

struct ClassAllocatorTest
{
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ClassAllocatorTest, 10u, 1024000);
};

int main()
{
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);
   size_t tmp = sizeof(ClassAllocatorTest);

   ClassAllocatorTest* tester = new ClassAllocatorTest();

   Foundation::HashName test("temp");

   delete tester;

   return 0;
}
