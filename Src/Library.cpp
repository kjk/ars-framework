#include <Library.hpp>

namespace ArsLexis
{

    Library::Library():
        refNum_(0),
        loaded_(false)
    {
    }
    
    Err Library::initialize(const char* name, UInt32 creator, UInt32 type)
    {
        assert(!refNum_);
        assert(!loaded_);
        Err error=SysLibFind(name, &refNum_);
        if (sysErrLibNotFound==error)
        {
            error=SysLibLoad(type, creator, &refNum_);
            if (!error)
                loaded_=true;
        }
        if (!error)
            assert(refNum_!=0);
        return error;
    }
    
    Library::~Library()
    {
        if (loaded_)
        {
            assert(refNum_!=0);
            SysLibRemove(refNum_);
        }
    }

}