#include "NetLibrary.hpp"

#define netLibName "Net.lib"

namespace ArsLexis
{

    NetLibrary::NetLibrary():
        closed_(false)
    {
    }
    
    Err NetLibrary::initialize(UInt16& ifError, UInt16 configIndex, UInt32 openFlags)
    {
        Err error=Library::initialize(netLibName, netCreator);
        if (!error)
            error=NetLibOpenConfig(refNum(), configIndex, openFlags, &ifError);
        if (netErrAlreadyOpen==error)
            error=errNone;
        return error;      
    }
    
    Err NetLibrary::close(Boolean immediate)
    {
        assert(!closed());
        Err error=NetLibClose(refNum(), immediate);
        if (!error)
            closed_=true;
        return error;
    }

    NetLibrary::~NetLibrary()
    {
        if (!closed())
            close();
    }

}