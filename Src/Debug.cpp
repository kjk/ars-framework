#include <Debug.hpp>

#ifdef free
# undef free
#endif
#ifdef malloc
# undef malloc
#endif
#ifdef new
# undef new
#endif
#ifdef delete
# undef delete
#endif


#ifdef _PALM_OS
# ifdef ARSLEXIS_USE_MEM_GLUE
#  include <MemGlue.h>
#  define MemPtrNew MemGluePtrNew
# endif
#else
# include <stdlib.h>
# define MemPtrNew malloc
# define MemPtrFree free
#endif

void* ArsLexis::allocate(size_t size)
{
    void* ptr=0;
    if (size) 
        ptr = MemPtrNew(size);
    else
        ptr = MemPtrNew(1);
    if (NULL == ptr)
        handleBadAlloc();
    return ptr;
}


void* operator new(size_t size)
{
    void* ptr = ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, 0, 0);
#endif            
    return ptr;
}

void* operator new(size_t size, const char* file, int line)
{
    void* ptr=ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

void operator delete(void *ptr)
{
    if (NULL != ptr) 
        MemPtrFree(ptr);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, 0, true, 0, 0);
#endif            
}

void operator delete[](void *ptr)
{
    ::operator delete(ptr);
}


void* operator new[](size_t size)
{
    void* ptr=ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, 0, 0);
#endif            
    return ptr;
}

void* operator new[](size_t size, const char* file, int line)
{
    void* ptr=ArsLexis::allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

void* malloc__(size_t size, const char* file, int line)
{
    if (0 == size)
        size = 1;
    void* ptr = MemPtrNew(size);
    if (NULL == ptr)
        return NULL;
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

void* malloc__(size_t size)
{
    if (0 == size)
        size = 1;
    void* ptr = MemPtrNew(size);
    if (NULL == ptr)
        return NULL;
    return ptr;
}

#if defined(_MSC_VER) && !defined(NDEBUG) && (_MSC_VER == 1400)

#include <altcecrt.h>

int __cdecl _CrtDbgReportW(int reason, const wchar_t* file, int line, const wchar_t* t2, const wchar_t* message, ...)
{
	return 1;
}

#endif
