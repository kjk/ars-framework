#ifndef __ARSLEXIS_UTILITY_HPP__
#define __ARSLEXIS_UTILITY_HPP__

#include "Debug.hpp"

namespace ArsLexis
{

    template<typename T>
    struct ObjectDeleter
    {
        void operator() (T* object) const 
        {
            delete object;
        }
    };

}

#endif