#include "ResolverConnection.hpp"
#include "NetLibrary.hpp"
#include "SocketAddress.hpp"
#include "Resolver.hpp"

namespace ArsLexis
{

    ResolverConnection::ResolverConnection(Resolver& resolver, SocketConnection* nextConnection, const String& address, UInt16 port, Resolver::DNS_Choice dnsChoice):
        SimpleSocketConnection(nextConnection->manager()),
        resolver_(resolver),
        nextConnection_(nextConnection),
        address_(address),
        port_(port),
        dnsChoice_(dnsChoice),
        requestId_(0),
        data_(0),
        resolvedAddress_(0)
    {
    }

    ResolverConnection::~ResolverConnection()
    {
        delete nextConnection_;
    }

    void ResolverConnection::open()
    {
        buildQuery();
        SimpleSocketConnection::open();
    }
    
    void ResolverConnection::handleError(Err error)
    {
        if (Resolver::dnsSecondary==dnsChoice_)
            resolver_.blockingResolveAndConnect(nextConnection_, address_, port_);
        else
            resolver_.doResolveAndConnect(nextConnection_, address_, port_, Resolver::dnsSecondary);
        nextConnection_=0;
        SimpleSocketConnection::handleError(error);
    }
    
    //! This structure is described by RFC1035 .
    //! @see http://www.freesoft.org/CIE/RFC/1035/40.htm
    struct DNS_Header
    {
        UInt16 id;
        union 
        {
            UInt16 attribsNum;
            struct 
            {
                UInt16 qr:1;
                UInt16 opcode:4;
                UInt16 aa:1;
                UInt16 tc:1;
                UInt16 rd:1;
                UInt16 ra:1;
                UInt16 z:3;
                UInt16 rcode:4;
            } attribs;
        };
        UInt16 qdcount;
        UInt16 ancount;
        UInt16 nscount;
        UInt16 arcount;
    };
    
    union DNS_HeaderUnion
    {
        DNS_Header header;
        UInt16 words[sizeof(DNS_Header)/2];
    };
    
    static void encodeQName(String& out, const String& qName)
    {
        UInt16 start=0;
        do 
        {
            String::size_type pos=qName.find(".", start);
            UInt16 end=(qName.npos==pos?qName.length():pos);
            String label(qName, start, end-start);
            //! @todo Add length checking for label (should be less than 64 bytes)
            char length=(UInt8)label.length();
            out+=length;
            out+=label;
            start=end+1;
        } while (start<qName.length());
        out+=(char)0;
    }
    
    void ResolverConnection::buildQuery()
    {
        DNS_HeaderUnion headerUnion;
        MemSet(&headerUnion, sizeof(headerUnion), 0);
        UInt16 headerLength=sizeof(headerUnion);
        headerUnion.header.id=requestId_=SysRandom(0);    // We don't care for id as we want to use TCP with recursion, not UDP
        headerUnion.header.attribs.qr=false;        // Query
        headerUnion.header.attribs.opcode=0;      // Standard query
        headerUnion.header.attribs.rd=true;         // Recursion is desired as we're using TCP
        headerUnion.header.qdcount=1;               // Only one query
        for (UInt16 i=0; i<sizeof(headerUnion)/2; ++i)
            headerUnion.words[i]=NetHToNS(headerUnion.words[i]);
        String request;
        request.append((const char*)&headerUnion, sizeof(headerUnion));
        encodeQName(request, address_);
        UInt16 qtype=NetHToNS(1);                    // QType=A (1)
        request.append((const char*)&qtype, 2);
        UInt16 qclass=NetHToNS(1);                  // QClass=IN (1)
        request.append((const char*)&qclass, 2); 
        UInt16 length=NetHToNS(request.length());
        request.insert(0, (const char*)&length, 2); // Prefix message with its length
        setRequest(request);
    }
    
    void ResolverConnection::finalize()
    {
        Err error=errNone;
        const char* data=response().data();
        UInt16 messageLength=NetNToHS((const UInt16&)(*data));
        if (response().length()==messageLength+2)
        {
            data+=2;
            data_=data;
            DNS_HeaderUnion& headerUnion=(DNS_HeaderUnion&)(*data);
            for (UInt16 i=0; i<sizeof(headerUnion)/2; ++i)
                headerUnion.words[i]=NetNToHS(headerUnion.words[i]);
            if (0!=headerUnion.header.attribs.rcode)
            {
                switch (headerUnion.header.attribs.rcode)
                {
                    case 1: 
                        error=netErrDNSFormat;
                        break;
                    case 2:
                        error=netErrDNSServerFailure;
                        break;
                    case 3:
                        error=netErrDNSNonexistantName;
                        break;
                    case 4:
                        error=netErrUnimplemented;
                        break;
                    case 5:
                        error=netErrDNSRefused;
                        break;
                    default:
                        error=netErrDNSNonexistantName;
                }                        
            }                
            if (!error)
            {                
                if (headerUnion.header.attribs.tc)
                    error=netErrDNSTruncated;
                else
                {
                    data+=sizeof(headerUnion);
                    for (UInt16 i=0; i<headerUnion.header.qdcount; ++i)
                        data+=StrLen(data)+5;           // Skip all QName entries (until null terminator, 2 words past it).
                    for (UInt16 i=0; i<headerUnion.header.ancount; ++i)
                        processAnswerRecord(data);
                    if (resolvedAddress_)
                    {
                        for (Aliases_t::const_iterator it=aliases_.begin(); it!=aliases_.end(); ++it)
                            resolver_.updateCacheEntry(*it, resolvedAddress_);
                        INetSocketAddress addr(resolvedAddress_, port_);
                        nextConnection_->setAddress(addr);
                        nextConnection_->open();
                        nextConnection_=0;
                    }
                    else
                        error=netErrDNSNonexistantName;
                }                        
            }                        
        }
        if (!error)
            SimpleSocketConnection::finalize();
        else
            handleError(error);            
    }
    
    String ResolverConnection::decodeName(const char*& data)
    {
        String result;
        while (*data)
        {
            UInt8 length=*data;
            ++data;
            if (0xc0 == (length & 0xc0))          // if length has 2 highest bits set it's pointer
            {
                UInt16 offset=((length & (~0xc0))<<8 | (UInt8)(*data));
                const char* base=data_+offset;
                if (!result.empty())
                    result+='.';
                result+=decodeName(base);
                break;
            }
            else
            {
                if (!result.empty())
                    result+='.';
                result.append(data, length);
                data+=length;
            }
        }
        ++data;
        return result;
    }
    
    void ResolverConnection::processAnswerRecord(const char*& data)
    {
        String name=decodeName(data);
        UInt16 qtype=NetNToHS((const UInt16&)(*data));
        data+=2;
        UInt16 qclass=NetNToHS((const UInt16&)(*data));
        data+=2;
        UInt32 ttl=NetNToHL((const UInt32&)(*data));
        data+=4;
        UInt16 rdlength=NetNToHS((const UInt16&)(*data));
        data+=2;
        if (1==qclass)
        {
            if (5==qtype)
                aliases_.push_front(name);
            else if (1==qtype && 4==rdlength)
            {
                NetIPAddr address=NetNToHL((const NetIPAddr&)(*data));
                resolvedAddress_=address;
                aliases_.push_front(name);
            }
        }            
        data+=rdlength;
    }
}