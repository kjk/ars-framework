#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#if __ide_target("Release")
#define NDEBUG
#endif

#ifndef NDEBUG
#define DEBUG
#endif


#include <new>

inline void * operator new(unsigned long size)
{
    if (size) 
        return MemPtrNew(size);
    return MemPtrNew(1);
}

inline void operator delete(void *ptr)
{
    if (ptr) 
        MemPtrFree(ptr);
}

inline void * operator new[](unsigned long size)
{
    if (size) 
        return MemPtrNew(size);
    return MemPtrNew(1);
}

inline void operator delete[](void *ptr)
{
    if (ptr) 
        MemPtrFree(ptr);
}

#include <cassert>

#endif