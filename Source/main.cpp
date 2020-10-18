#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>

int main()
{
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);

   return 0;
}
