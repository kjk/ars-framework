#ifndef __ARSLEXIS_UTILITY_HPP__
#define __ARSLEXIS_UTILITY_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis
{

    template<class T> 
    struct ObjectDeleter
    {
        void operator() (T* object) const 
        {
            delete object;
        }
    };
    
    class NonCopyable
    {
        explicit NonCopyable(const NonCopyable&);
        NonCopyable& operator=(const NonCopyable&);
    public:
        NonCopyable() {}        
    };  
    
}

#endif