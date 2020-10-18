#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>

#include <Util/HashName.h>

int main()
{
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);

   Foundation::HashName test("temp");

   return 0;
}
