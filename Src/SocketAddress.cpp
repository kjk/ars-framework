#include <SocketAddress.hpp>
#include <cstring>

namespace ArsLexis
{

    SocketAddress::SocketAddress()
    {
        using namespace std;
        std::memset(&address_, sizeof(address_), 0);
    }

    INetSocketAddress::INetSocketAddress(const NativeIPAddr_t& ipAddr, ushort_t port, short addressFamily)
    {
        address().family=addressFamily;
        address().port=netToHostS(port);
        address().ip=ipAddr;
    }
    
    INetSocketAddress::INetSocketAddress()
    {
        address().family=socketAddrINET;
        address().port=0;
        address().ip=0;
    }
    
}