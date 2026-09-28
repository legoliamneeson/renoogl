"""Exercise the actual allocator functions with a mock Windows process heap.
The native alignment is simulated as 8 and 16 bytes. This is not a Windows/x86 build.
"""
from pathlib import Path
import subprocess
import tempfile
source = (Path(__file__).resolve().parents[2] / 'src/utils/platform.hpp').read_text()
source = source[source.index('inline void* ProcessHeapAlignedAlloc('):source.index('template <typename T>\nstruct ProcessSharedSlot')]
prefix = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <utility>
#include <unordered_set>
using DWORD=unsigned long;
constexpr DWORD HEAP_ZERO_MEMORY=8;
std::unordered_set<void*> allocations;
void* GetProcessHeap(){return nullptr;}
void* HeapAlloc(void*,DWORD flags,size_t size){auto p=std::malloc(size);if(p){allocations.insert(p);if(flags&HEAP_ZERO_MEMORY)std::memset(p,0,size);}return p;}
bool HeapFree(void*,DWORD,void* p){assert(allocations.erase(p)==1);std::free(p);return true;}
'''
suffix = r'''
struct alignas(64) Item { int value; explicit Item(int v=17):value(v){} };
int main(){
 for(size_t alignment: {size_t(8),size_t(16),size_t(32),size_t(64),size_t(256)})
  for(size_t size: {size_t(1),size_t(16),size_t(257)}){
   auto* p=static_cast<unsigned char*>(ProcessHeapAlignedAlloc(size,alignment,HEAP_ZERO_MEMORY));
   assert(p && reinterpret_cast<uintptr_t>(p)%alignment==0);
   for(size_t i=0;i<size;i++)assert(p[i]==0);
   std::memset(p,0x7f,size);ProcessHeapAlignedFree(p,alignment);
  }
 assert(!ProcessHeapAlignedAlloc(std::numeric_limits<size_t>::max(),64));
 assert(!ProcessHeapAlignedAlloc(32,48));
 ProcessHeapAlignedFree(nullptr,64);
 ProcessAllocator<Item> a;auto p=a.allocate(3);assert(reinterpret_cast<uintptr_t>(p)%64==0);a.deallocate(p,3);
 bool rejected=false;try{auto p=a.allocate(std::numeric_limits<size_t>::max());a.deallocate(p,0);}catch(const std::bad_array_new_length&){rejected=true;}assert(rejected);
 auto obj=CreateSharedObject<Item>(23);assert(obj->value==23);assert(reinterpret_cast<uintptr_t>(obj)%64==0);DeleteSharedObject(obj);
 assert(allocations.empty());
}
'''
with tempfile.TemporaryDirectory() as tmp:
    for native in (8,16):
        cpp=Path(tmp)/'test.cpp';exe=Path(tmp)/'test'
        cpp.write_text(prefix+source.replace('sizeof(void*) == 4 ? 8u : 16u',str(native)+'u')+suffix)
        subprocess.run(['g++','-std=c++20','-Wall','-Wextra',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
        print(f'Process heap tests passed: simulated native alignment {native}')
