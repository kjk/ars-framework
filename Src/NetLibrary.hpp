#ifndef __ARSLEXIS_NETLIBRARY_HPP__
#define __ARSLEXIS_NETLIBRARY_HPP__

#include <Library.hpp>

namespace ArsLexis 
{

    class INetSocketAddress;

    class NetLibrary: private Library
    {
        bool closed_:1;
        bool libraryOpened_:1;
        
    public:
        
        NetLibrary();
        
        ~NetLibrary();
        
        enum {
            defaultConfig
        };
        
        operator UInt16() const
        {return Library::refNum();}
        
        Err initialize(UInt16& ifError, UInt16 configIndex=defaultConfig, UInt32 openFlags=0);
        
        bool closed() const 
        {return closed_;}
        
        Err close(bool immediate=false);
        
        Err getSetting(UInt16 setting, void* value, UInt16& valueLength) const;
        Err setSetting(UInt16 setting, const void* value, UInt16 valueLength);
        
        Err getHostByName(const char* name, NetHostInfoBufType* buffer, Int32 timeout=evtWaitForever);
        
        Err addrAToIN(const char* addr, INetSocketAddress& out);
        
    };
    
}

#endif