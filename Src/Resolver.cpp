#include <Resolver.hpp>
#include <NetLibrary.hpp>
#include <cctype>
#include <Text.hpp>

namespace ArsLexis
{

    Resolver::Resolver(NetLibrary& netLib):
        netLib_(netLib)
    {
        updateCacheEntry("localhost", 0x7f000001);
    }

    Resolver::~Resolver()
    {}
    
/*    
    namespace {
    
        struct CacheEntryComparator {
            
            const String& text;
            
            CacheEntryComparator(const String& t): text(t) {}
            
            bool operator()(const Resolver::CacheEntry_t& entry) const 
            {return entry.first==text;}
            
        };
    
    }
*/
    
    void Resolver::updateCacheEntry(const String& name, NetIPAddr address)
    {
/*    
        AddressCache_t::iterator it=std::find_if(cache_.begin(), cache_.end(), CacheEntryComparator(name));
        if (it!=cache_.end())
            (*it).second=address;
        else
            cache_.push_front(std::make_pair(name, address));
*/
        cache_[name]=address;            
    }

    Err Resolver::validateAddress(const String& origAddress, String& validAddress, UInt16& port)
    {
        Err error=errNone;
        String::size_type pos=origAddress.find(':', 1);
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
   
    Err Resolver::blockingResolve(SocketAddress& out, const String& name, UInt16 port, UInt32 timeout)
    {
        std::auto_ptr<NetHostInfoBufType> buffer(new NetHostInfoBufType);
        MemSet(buffer.get(), sizeof(NetHostInfoBufType), 0);
        assert(!netLib_.closed());
        Err error=netLib_.getHostByName(name.c_str(), buffer.get(), timeout);
        if (error)
            return error;

        NetIPAddr resAddr=buffer->address[0];
        assert(resAddr!=0);
//        cache_.push_front(std::make_pair(name, resAddr));
        cache_[name]=resAddr;

#ifdef NEVER
        char addrStr[32];
        NetLibAddrINToA(netLib_.refNum(), resAddr, addrStr);
        ChildLogger log("Resolver");
        log().info()<< "Resolver::blockingResolveAndConnect to ip="<<addrStr;
#endif
        INetSocketAddress addr(resAddr, port);
        out=addr;
        return errNone;
    }
   
    Err Resolver::resolve(SocketAddress& out, const String& address, UInt16 port, UInt32 timeout)
    {
        String validAddress;                
        Err error=validateAddress(address, validAddress, port);
        if (error)
            return error;
        INetSocketAddress addr;
        if (!validAddress.empty() && isDigit(validAddress[0]))
        {
            error=netLib_.addrAToIN(validAddress.c_str(), addr);
            if (error)
                return error;
            addr.setPort(port);
            out=addr;
            return errNone;
        }
        AddressCache_t::const_iterator it=cache_.find(validAddress);
//        AddressCache_t::const_iterator it=std::find_if(cache_.begin(), cache_.end(), CacheEntryComparator(validAddress));
        if (it!=cache_.end())
        {
            addr.setIpAddress(it->second);
            addr.setPort(port);
            out=addr;
            return errNone;
        }
        else
            return blockingResolve(out, validAddress, port, timeout);
    }
   
}
