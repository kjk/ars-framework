/**
 * @file Debug.hpp
 * Debugging enhancements etc.
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_DEBUG_HPP__
#define __ARSLEXIS_DEBUG_HPP__

#if defined(__MWERKS__)

#if __ide_target("Release") && !defined(NDEBUG)
#define NDEBUG
#endif

#ifndef NDEBUG
//! Some functions depend on this non-standard symbol instead of standard-compliant @c NDEBUG.
#define DEBUG
#endif

//! Prevents using MSL-provided error function (called when normally exception would be thrown).
#define _MSL_ERROR_FUNC -1

#endif // __MWERKS__

#include <new>
#include <cassert>

namespace ArsLexis 
{
    /** 
     * Placeholder for custom memory allocation failure handler.
     * It should be defined somewher in application modules.
     */
    void handleBadAlloc();
    
    void logAllocation(void* ptr, bool free);
    
}

/**
 * Custom memory allocation function, that doesn't throw (contrary 
 * to MSL new (nothrow) that simply catches exception
 * thrown by new.
 */
inline void * operator new(unsigned long size)
{
    void* ptr=0;
    if (size) 
        ptr=MemPtrNew(size);
    else
        ptr=MemPtrNew(1);
    if (!ptr)
        ArsLexis::handleBadAlloc();
#ifndef NDEBUG
    else
        ArsLexis::logAllocation(ptr, false);
#endif                
    return ptr;
}

inline void operator delete(void *ptr)
{
    if (ptr) 
    {
        MemPtrFree(ptr);
#ifndef NDEBUG
        ArsLexis::logAllocation(ptr, true);
#endif            
    }        
}

inline void * operator new[](unsigned long size)
{
    return ::operator new(size);
}

inline void operator delete[](void *ptr)
{
    ::operator delete(ptr);
}

#endif