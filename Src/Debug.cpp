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
#ifdef realloc
# undef realloc
#endif


#ifdef _PALM_OS
# ifdef ARSLEXIS_USE_MEM_GLUE
#  include <MemGlue.h>
#  define malloc MemGluePtrNew
# else
#  define malloc MemPtrNew
# endif
# define free MemPtrFree
#else
# include <stdlib.h>
#endif

static void* allocate(size_t size)
{
    void* ptr=0;
    if (size) 
        ptr = malloc(size);
    else
        ptr = malloc(1);
    if (NULL == ptr)
        ArsLexis::handleBadAlloc();
    return ptr;
}


void* operator new(size_t size, const char* file, int line)
{
    void* ptr = allocate(size);
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

void* operator new(size_t size)
{
    return ::operator new(size, __FILE__, __LINE__);
}

void operator delete(void *ptr)
{
    if (NULL != ptr) 
        free(ptr);
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
    return ::operator new(size, __FILE__, __LINE__);
}

void* operator new[](size_t size, const char* file, int line)
{
    return ::operator new(size, file, line);
}

void* malloc__(size_t size, const char* file, int line)
{
    if (0 == size)
        size = 1;
    void* ptr = malloc(size);
    if (NULL == ptr)
        return NULL;
#ifndef NDEBUG
    ArsLexis::logAllocation(ptr, size, false, file, line);
#endif            
    return ptr;
}

void* realloc__(void* p, size_t size, const char* file, int line)
{
    if (NULL != p && 0 == size)
    {
        free__(p);
        return NULL;
    }
    if (NULL == p)
        return malloc__(size, file, line);

#ifdef _PALM_OS

    if (errNone == MemPtrResize(p, size))
        return p;

    void* np = malloc__(size, file, line);
    if (NULL == np)
        return NULL;

    MemMove(np, p, MemPtrSize(p));
    free__(p);

#elif defined(_WIN32_WCE)

    void* np = realloc(p, size);
#ifndef NDEBUG    
    if (np != p)
    {
        logAllocation(p, 0, true, __FILE__, __LINE__);
        if (NULL != np)
            logAllocation(np, size, false, __FILE__, __LINE__);
    }
#endif    

#endif

    return np;
}

void* realloc__(void* p, size_t s) 
{
    return realloc__(p, s, __FILE__, __LINE__);
}

void* malloc__(size_t size)
{
    return malloc__(size, __FILE__, __LINE__);
}

#if defined(_MSC_VER) && !defined(NDEBUG) && (_MSC_VER == 1400)

#include <altcecrt.h>

int __cdecl _CrtDbgReportW(int reason, const wchar_t* file, int line, const wchar_t* t2, const wchar_t* message, ...)
{
	return 1;
}

#endif

#if defined(_MSC_VER)

void ArsLexis::handleBadAlloc()
{
#if _MSC_VER == 1200
    RaiseException(memErrNotEnoughSpace, 0, 0, NULL); 
#else
	throw long(memErrNotEnoughSpace);
#endif
}

#endif