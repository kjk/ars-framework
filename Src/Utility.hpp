#ifndef __ARSLEXIS_UTILITY_HPP__
#define __ARSLEXIS_UTILITY_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

template<class T> 
struct ObjectDeleter
{
    T* operator() (T* object) const 
    {
        delete object;
        return 0;
    }
};

template<bool b=false>
struct StaticAssert;

template<>
struct StaticAssert<true> {
};

class NonCopyable
{
    explicit NonCopyable(const NonCopyable&);
    NonCopyable& operator=(const NonCopyable&);
public:
    NonCopyable() {}        
};

template<class T>
inline void PassOwnershipP(T** source, T** target)
{
    delete *target;
    *target = *source;
    *source = NULL;
}

template<class T>
inline void PassOwnership(T*& source, T*& target)
{
    delete target;
    target = source;
    source = NULL;
}

template<class T> 
inline void Zero(T& var) {
	ZeroMemory(&var, sizeof(var));
}



#endif
