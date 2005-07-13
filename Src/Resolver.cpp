#include <Resolver.hpp>
#include <NetLibrary.hpp>
#include <Text.hpp>
#include <BaseTypes.hpp>

#ifdef __MWERKS__
# pragma far_code
#endif

static status_t validateAddress(const NarrowString& origAddress, NarrowString& validAddress, ushort_t& port)
{
    NarrowString::size_type pos = origAddress.find(':', 1);
    if (origAddress.npos==pos)
        return netErrParamErr;
    
    ushort_t portLength = origAddress.length()-pos-1;
    if (portLength>0)
    {
        long value=0;
        status_t error = numericValue(origAddress.data()+pos+1, origAddress.data()+pos+1+portLength, value);
        if ((errNone != error) || (value > ushort_t(-1)))
            return netErrParamErr;

        port = ushort_t(value);
    }

    if (0==port)
        return netErrParamErr;        

    validAddress.assign(origAddress, 0, pos);
    return errNone;
}

static status_t blockingResolve(SocketAddress& out, NetLibrary& netLib, const NarrowString& name, ushort_t port, ulong_t timeout)
{
    std::auto_ptr<HostInfoBuffer> buffer(new  HostInfoBuffer);
    memzero(buffer.get(), sizeof(HostInfoBuffer));
    assert(!netLib.closed());
    status_t error=netLib.getHostByName(name.c_str(), *buffer, timeout);
    if (error)
        return error;

    IPAddr  resAddr=buffer->getAddress();
    assert(resAddr.ip != 0);
    INetSocketAddress addr(resAddr, port);
    out=addr;
    return errNone;
}

status_t resolve(SocketAddress& out, NetLibrary& netLib, const char* address, ushort_t port, ulong_t timeout)
{
    NarrowString validAddress;
    NarrowString addressStr(address);
    status_t error=validateAddress(address, validAddress, port);
    if (error)
        return error;
    INetSocketAddress addr;
    // TODO: the assumption that the ip address is numeric if the first
    // letter is a digit is incorrect. A symbolic name can also start
    // with a digit. Better test would be to check if the whole name
    // consists of digits or "."
    if (!validAddress.empty() && isDigit(validAddress[0]))
    {
        error=netLib.addrAToIN(validAddress.c_str(), addr);
        if (error)
            return error;
        addr.setPort(port);
        out=addr;
        return errNone;
    }
    return blockingResolve(out, netLib, validAddress, port, timeout);
}

