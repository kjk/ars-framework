/**
 * @file Debug.hpp
 * Debugging enhancements etc.
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_DEBUG_HPP__
#define __ARSLEXIS_DEBUG_HPP__

#if defined(__MWERKS__)

//! Prevents using MSL-provided error function (called when normally exception would be thrown).
#define _MSL_ERROR_FUNC -1

#endif // __MWERKS__

#if defined(_PALM_OS)

#include <new>          // I include <new> & <memory> here so that definitions from there are included before I redefine new
#include <memory>
#include <cassert>
#include <string>

namespace ArsLexis 
{
    /** 
     * Placeholder for custom memory allocation failure handler.
     * It should be defined somewher in application modules.
     */
    void handleBadAlloc();
    
    void logAllocation(void* ptr, bool free, const char* file, int line);
    
}

/**
 * Custom memory allocation function, that doesn't throw (contrary 
 * to MSL new (nothrow) that simply catches exception
 * thrown by new.
 */
inline void* operator new(unsigned long size)
{
    void* ptr=0;
    if (size) 
        ptr=MemPtrNew(size);
    else
        ptr=MemPtrNew(1);
    if (!ptr)
        ArsLexis::handleBadAlloc();
    return ptr;
}

inline void* operator new(unsigned long size, const char* file, int line)
{
    void* ptr=::operator new(size);
    ArsLexis::logAllocation(ptr, false, file, line);
    return ptr;
}

inline void operator delete(void *ptr)
{
    if (ptr) 
    {
        MemPtrFree(ptr);
#ifndef NDEBUG
        ArsLexis::logAllocation(ptr, true, 0, 0);
#endif            
    }        
}

inline void* operator new[](unsigned long size)
{
    return ::operator new(size);
}

inline void* operator new[](unsigned long size, const char* file, int line)
{
    void* ptr=::operator new[](size);
    ArsLexis::logAllocation(ptr, false, file, line);
    return ptr;
}

inline void operator delete[](void *ptr)
{
    ::operator delete(ptr);
}

#ifndef NDEBUG
#define new new (__FILE__, __LINE__)
#endif

#endif // _PALM_OS

#endif