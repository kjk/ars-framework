#ifndef __ARSLEXIS_NETLIBRARY_HPP__
#define __ARSLEXIS_NETLIBRARY_HPP__

#include <Library.hpp>

namespace ArsLexis 
{

    class INetSocketAddress;

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
        
        Err getSetting(UInt16 setting, void* value, UInt16& valueLength) const;
        Err setSetting(UInt16 setting, const void* value, UInt16 valueLength);
        
        Err getHostByName(const char* name, NetHostInfoBufType* buffer, Int32 timeout=evtWaitForever);
        
        Err addrAToIN(const char* addr, INetSocketAddress& out);
        
    };
    
}

#endif