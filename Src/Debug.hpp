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


#include <new>          // I include <new> & <memory> here so that definitions from there are included before I redefine new
#include <memory>

#if defined(__MWERKS__)
using std::size_t;
# include <cassert>
#endif

#include <BaseTypes.hpp>


namespace ArsLexis 
{
    /** 
     * Placeholder for custom memory allocation failure handler.
     * It should be defined somewher in application modules.
     */
    void handleBadAlloc();
    
    void logAllocation(void* ptr, size_t size, bool free, const char* file, int line);
    
    void cleanAllocationLogging();
    
    void* allocate(size_t size);

}

/**
 * Custom memory allocation function, that doesn't throw (contrary 
 * to MSL new (nothrow) that simply catches exception
 * thrown by new.
 */
inline void* operator new(size_t size)
{
    void* ptr=ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, 0, 0);
#endif            
    return ptr;
}

inline void* operator new(size_t size, const char* file, int line)
{
    void* ptr=ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

inline void operator delete(void *ptr)
{
    if (ptr) 
        free(ptr);
#ifndef NDEBUG
        ArsLexis::logAllocation(ptr, 0, true, 0, 0);
#endif            
}

inline void operator delete(void* ptr, const char*, int)
{
    ::operator delete(ptr);
}

inline void* operator new[](size_t size)
{
    void* ptr=ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, 0, 0);
#endif            
    return ptr;
}

inline void* operator new[](size_t size, const char* file, int line)
{
    void* ptr=ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

inline void operator delete[](void *ptr)
{
    ::operator delete(ptr);
}

inline void operator delete[](void *ptr, const char*, int line)
{
    ::operator delete(ptr);
}

inline void* malloc__(size_t size, const char* file, int line)
{
#ifdef _PALM_OS
    if (0 == size)
        size = 1;
    void* ptr = MemGluePtrNew(size);
#else
    void* ptr = malloc(size);
#endif
    if (NULL == ptr)
        return NULL;
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

inline void free__(void* p)
{
    ::operator delete(p);
}

#ifdef malloc
# undef malloc
#endif
#ifdef free
# undef free
#endif

#define malloc(a) malloc__((a), __FILE__, __LINE__)
#define free(a) free__(a)

#if !defined(NDEBUG) && !defined(_MSC_VER)
// MS VC++ containers use operator placement new to construct values (instead of allocator::construct()).
# define new new (__FILE__, __LINE__)
#endif

#endif