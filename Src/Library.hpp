#ifndef __ARSLEXIS_LIBRARY_HPP__
#define __ARSLEXIS_LIBRARY_HPP__

#include <Debug.hpp>
#include <Utility.hpp>

class Library: private NonCopyable
{
    UInt16 refNum_;
    bool loaded_;
    
public:

    Library();

    virtual ~Library();
    
    Err initialize(const char* name, UInt32 creator, UInt32 type=sysFileTLibrary);

    UInt16 refNum() const
    {return refNum_;}
    
    operator UInt16 () const 
    {return refNum();}
    
};

#endif