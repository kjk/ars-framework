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

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#include <cassert>
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

enum NewDontThrowTag {newDontThrow};

/**
 * Custom memory allocation function, that doesn't throw (contrary 
 * to MSL new (nothrow) that simply catches exception
 * thrown by new.
 */
void* operator new(size_t size);

void* operator new(size_t size, const char* file, int line);

void operator delete(void *ptr);

void* operator new[](size_t size);

void* operator new[](size_t size, const char* file, int line);

void operator delete[](void *ptr);

void* malloc__(size_t size, const char* file, int line);

void* malloc__(size_t size);

inline void free__(void* p)
{
    ::operator delete(p);
}

inline void* operator new(size_t size, NewDontThrowTag)
{
    return malloc__(size);
}

inline void* operator new(size_t size, NewDontThrowTag, const char* file, int line)
{
    return malloc__(size, file, line);
}

inline void* operator new[](size_t size, NewDontThrowTag)
{
    return malloc__(size);
}

inline void* operator new[](size_t size, NewDontThrowTag, const char* file, int line)
{
    return malloc__(size, file, line);
}

inline void operator delete(void* p, NewDontThrowTag)
{
    ::operator delete(p);
}

inline void operator delete[](void* p, NewDontThrowTag)
{
    ::operator delete(p);
}

inline void operator delete(void* p, NewDontThrowTag, const char*, int)
{
    ::operator delete(p);
}

inline void operator delete[](void* p, NewDontThrowTag, const char*, int)
{
    ::operator delete(p);
}

#ifdef malloc
# undef malloc
#endif
#ifdef free
# undef free
#endif

#define free(a) free__(a)

#if !defined(NDEBUG) && !defined(_MSC_VER)
// MS VC++ containers use operator placement new to construct values (instead of allocator::construct()).
# define malloc(a) malloc__((a), __FILE__, __LINE__)
# define new new (__FILE__, __LINE__)
# define new_nt new //(newDontThrow, __FILE__, __LINE__)
#else
# define new_nt new (newDontThrow)
# define malloc(a) malloc__(a)
#endif

using namespace ArsLexis;

#endif