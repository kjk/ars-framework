#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#if __ide_target("Release")
#define NDEBUG
#endif

#ifndef NDEBUG
#define DEBUG
#endif

// Prevents using MSL-provided error function (called when normally exception would be thrown).
#define _MSL_ERROR_FUNC -1

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