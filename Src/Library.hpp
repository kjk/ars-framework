#ifndef __ARSLEXIS_LIBRARY_HPP__
#define __ARSLEXIS_LIBRARY_HPP__

#include "Debug.hpp"

namespace ArsLexis 
{

    class Library
    {
        UInt16 refNum_;
        Boolean loaded_;
        
        Library(const Library&);
        Library& operator=(const Library&);
        
    public:
    
        Library();

        virtual ~Library();
        
        Err initialize(const char* name, UInt32 creator, UInt32 type='libr');

        UInt16 refNum() const
        {return refNum_;}
        
        operator UInt16 () const 
        {return refNum();}
        
    };

}

#endif