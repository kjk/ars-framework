/**
 * @file Debug.hpp
 * Debugging enhancements etc.
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_DEBUG_HPP__
#define __ARSLEXIS_DEBUG_HPP__

//#pragma warn_a5_access on

#if __ide_target("Release")
#define NDEBUG
#endif

#ifndef NDEBUG
//! Some functions depend on this non-standard symbol instead of standard-compliant @c NDEBUG.
#define DEBUG
#endif

//! Prevents using MSL-provided error function (called when normally exception would be thrown).
#define _MSL_ERROR_FUNC -1

#include <new>
#include <cassert>

namespace ArsLexis 
{
    /** 
     * Placeholder for custom memory allocation failure handler.
     * It should be defined somewher in application modules.
     */
    void handleBadAlloc();
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
    return ptr;
}

inline void operator delete(void *ptr)
{
    if (ptr) 
        MemPtrFree(ptr);
}

inline void * operator new[](unsigned long size)
{
    return ::operator new(size);
}

inline void operator delete[](void *ptr)
{
    ::operator delete(ptr);
}

#include <string>

namespace ArsLexis 
{

    /**
     * Replacement for @c std::allocator. The former one uses (not explicitely) some 
     * globals.
     */
    template <class T> 
    class Allocator
    {
    public:
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        typedef T*        pointer;
        typedef const T*  const_pointer;
        typedef T&        reference;
        typedef const T&  const_reference;
        typedef T         value_type;

        template <class U> 
        struct rebind { typedef Allocator<U> other; };

        Allocator()
        {}

        template <class U> inline
        Allocator(const Allocator<U>&) 
        {}

        pointer address(reference x) const
        {return &x;}

        const_pointer address(const_reference x) const
        {return &x;}

        pointer allocate(size_type n, const_pointer hint = 0)
        {
            return static_cast<pointer>(::operator new(n*sizeof(T)));
        }

        void deallocate(pointer p, size_type n)
        {
            ::operator delete(p);
        }

        size_type max_size() const
        {
            return ((size_type)-1)/sizeof(T);
        }

        void construct(pointer p, const T& val)
        {
            new (p) T(val);
        }

        void destroy(pointer p)
        {
            p->T::~T();
        }
        
    };

    typedef std::basic_string<char, std::char_traits<char>, Allocator<char> > String;
}

#endif