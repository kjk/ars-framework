#include "Resolver.hpp"
#include "ResolverConnection.hpp"
#include "SysUtils.hpp"
#include "NetLibrary.hpp"

namespace ArsLexis
{

    void Resolver::updateCacheEntry(const String& name, NetIPAddr address)
    {
        for (int i=0; i<dnsAddressesCount_; ++i)
            dnsAddresses_[i]=0;
        cache_[name]=address;         
        queryServerAddresses();   
    }

    Resolver::Resolver(NetLibrary& netLib):
        netLib_(netLib)
    {
        updateCacheEntry("localhost", 0x7f000001);
    }
    
    Resolver::~Resolver()
    {
    }
    
    UInt32 Resolver::dnsAddress(DNS_Choice choice)
    {
        assert(choice<dnsAddressesCount_);
        return dnsAddresses_[choice];
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
    
    void Resolver::doResolveAndConnect(SocketConnection* connection, const String& name, UInt16 port, DNS_Choice choice)
    {
        UInt32 dnsAddr=dnsAddress(choice);
        if (0==dnsAddr)
        {
            if (dnsPrimary==choice)
                doResolveAndConnect(connection, name, port, dnsSecondary);
            else
                blockingResolveAndConnect(connection, name, port);
        }
        else
        {
            ResolverConnection* resolverConnection=new ResolverConnection(*this, connection, name, port, choice);
            INetSocketAddress addr(dnsAddr, 53);
            resolverConnection->setAddress(addr);
            resolverConnection->open();
        }            
    }
    
    void Resolver::blockingResolveAndConnect(SocketConnection* connection, const String& name, UInt16 port)
    {
        NetHostInfoBufType* buffer=new NetHostInfoBufType;
        MemSet(buffer, sizeof(*buffer), 0);
        Err error=netLib_.getHostByName(name.c_str(), buffer, connection->transferTimeout());
        if (!error)
        {
            NetIPAddr resAddr=buffer->address[0];
            assert(resAddr!=0);
            INetSocketAddress addr(resAddr, port);
            connection->setAddress(addr);
            connection->open();
        }
        else
            connection->handleError(error);
        delete buffer;
    }
   
    Err Resolver::resolveAndConnect(SocketConnection* connection, const String& address, UInt16 port)
    {
        assert(connection!=0);
        String validAddress;                
        Err error=validateAddress(address, validAddress, port);
        if (!error)
        {
            AddressCache_t::const_iterator it=cache_.find(validAddress);
            if (it!=cache_.end())
            {
                INetSocketAddress addr(it->second, port);
                connection->setAddress(addr);
                connection->open();
            }
            else
                doResolveAndConnect(connection, validAddress, port, dnsPrimary);
        }
        return error;            
    }
   
    void Resolver::queryServerAddresses()
    {
        static const UInt16 dnsPort=53;
        for (int i=0; i<dnsAddressesCount_; ++i)
        {
            UInt16 addressSize=sizeof(NetIPAddr);
            Err error=netLib_.getSetting(i>0?netSettingSecondaryDNS:netSettingPrimaryDNS, &dnsAddresses_[i], addressSize);
        }        
    }

}