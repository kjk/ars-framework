#ifndef __ARSLEXIS_NETLIBRARY_HPP__
#define __ARSLEXIS_NETLIBRARY_HPP__

#include "Library.hpp"

namespace ArsLexis 
{

    class NetLibrary: public Library
    {
        Boolean closed_;
        
    public:
        
        NetLibrary();
        
        ~NetLibrary();
        
        enum {
            defaultConfig
        };
        
        Err initialize(UInt16& ifError, UInt16 configIndex=defaultConfig, UInt32 openFlags=0);
        
        Boolean closed() const 
        {return closed_;}
        
        Err close(Boolean immediate=false);
    
    };
    
}

#endif