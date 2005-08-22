#include <Lock.hpp>

#ifdef _WIN32

CriticalSection::CriticalSection()
{
    InitializeCriticalSection(&section_);
}

CriticalSection::~CriticalSection()
{
    DeleteCriticalSection(&section_);
}

void CriticalSection::acquire()
{
    EnterCriticalSection(&section_);
}

void CriticalSection::release()
{
    LeaveCriticalSection(&section_);
}

#endif // _WIN32