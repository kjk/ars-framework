#include "Resolver.hpp"
#include "ResolverConnection.hpp"
#include "SysUtils.hpp"

namespace ArsLexis
{

    void Resolver::updateCacheEntry(const String& name, UInt32 address)
    {
        AddressCache_t::iterator it=std::find_if(cache_.begin(), cache_.end(), CacheEntryComparator(name));
        if (it!=cache_.end())
            it->address=address;
        else
            cache_.push_front(CacheEntry(name, address));
    }

    Resolver::Resolver()
    {
        updateCacheEntry("localhost", 0x7f000001);
    }
    
    Resolver::~Resolver()
    {
    }
    
    Err Resolver::validateAddress(const String& origAddress, String& validAddress, UInt16& port)
    {
        Err error=errNone;
        String::size_type pos=origAddress.find(":");
        if (origAddress.npos!=pos)
        {
            UInt16 portLength=origAddress.length()-pos-1;
            if (portLength>0)
            {
                Int32 value=0;
                error=numericValue(origAddress.data()+pos+1, origAddress.data()+pos+1+portLength, value);
                if (!error && value<(UInt16)-1)
                    port=value;
                else
                    error=netErrParamErr;                    
            }
            else
                error=netErrParamErr;
        }
        if (port==0)
            error=netErrParamErr;        
        if (!error)
            validAddress.assign(origAddress, 0, pos);
        return error;
    }
    
   
    Err Resolver::resolveAndConnect(SocketConnection* connection, const String& address, UInt16 port)
    {
        assert(connection!=0);
        String validAddress;                
        Err error=validateAddress(address, validAddress, port);
        if (!error)
        {
            AddressCache_t::const_iterator it=std::find_if(cache_.begin(), cache_.end(), CacheEntryComparator(validAddress));
            if (it!=cache_.end())
            {
                INetSocketAddress addr(it->address, port);
                connection->setAddress(addr);
                connection->open();
            }
            else
            {
                ResolverConnection* resolverConnection=new ResolverConnection(*this, connection, validAddress, port);
                resolverConnection->open();
            }
        }
        return error;            
    }
   


}