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
    
    template<bool b=false>
    struct StaticAssert;
    
    template<>
    struct StaticAssert<true> {
    };
    
    template<class To, class From>
    inline To safe_reinterpret_cast(From from)
    {
        static ArsLexis::StaticAssert<sizeof(From)<=sizeof(To)> size_of_From_less_than_size_of_To;
        return reinterpret_cast<To>(from);
    }
    
}


#endif