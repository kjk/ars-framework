#ifndef __ARSLEXIS_HASHTABLE_HPP__
#define __ARSLEXIS_HASHTABLE_HPP__

#include "BaseTypes.hpp"

namespace ArsLexis {
    class Hashtable
    {
        char_t **entity_;
        char_t **value_;
        int counter_;
        int max_;

    public:
        Hashtable();

        void put(const String& entity, const String& value);

        String get(const String& code) const;
        
        //Use it to free all private memory of Hashtable
        void Dispose();
        
        ~Hashtable() 
        {Dispose();}
    };
}
#endif
