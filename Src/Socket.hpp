/**
 * @file Socket.hpp
 * C++ wrappers for PalmOS sockets implementation.
 * 
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_SOCKET_HPP__
#define __ARSLEXIS_SOCKET_HPP__

#include "Debug.hpp"

namespace ArsLexis
{

    class NetLibrary;
    
    class SocketAddress;

    /**
     * Base class for @c Socket and @c ServerSocket.
     * Implements functionality common to both socket types. Can't be instantiated stand-alone,
     * as it wouldn't make much sense anyway.
     */
    class SocketBase
    {
        /**
         * @internal
         * Undefined. Prevents copying.
         */
        SocketBase(const SocketBase&);

        /**
         * @internal
         * Undefined. Prevents copying.
         */
        SocketBase& operator=(const SocketBase&);

    protected:
    
        /**
         * Stores reference to currently loaded NetLib.
         */
        NetLibrary& netLib_;

        /**
         * Handle to underlying PalmOS socket.
         */
        NetSocketRef socket_;

        /**
         * Constructor for use by subclasses. 
         * As it's protected, you can't instantiate this class directly.
         * @param netLib reference to @c NetLibrary object that will be used to gain access to network services.
         */
        SocketBase(NetLibrary& netLib);
        
        /**
         * Closes associated socket handle and releases any other system resources.
         * As it's protected, you can't delete the @c SocketBase directly. This way we 
         * don't have to make destructor virtual.
         */
        ~SocketBase();
                
    public:
        
        /**
         * Opens (creates) the socket.
         * @see NetLibSocketOpen()
         */
        Err open(NetSocketAddrEnum domain=netSocketAddrINET,
            NetSocketTypeEnum type=netSocketTypeStream,
            Int16 protocol=0, Int32 timeout=evtWaitForever);

        Err shutdown(Int16 direction, Int32 timeout=evtWaitForever);
        
        Err send(UInt16& sent, const void* buffer, UInt16 bufferLength, Int32 timeout=evtWaitForever, UInt16 flags=0, const SocketAddress* address=0);
        
        Err receive(UInt16& received, void* buffer, UInt16 bufferLength, Int32 timeout=evtWaitForever, UInt16 flags=0);
        
        operator NetSocketRef() const
        {return socket_;}
        
    };

    /**
     * Implementation of client socket.
     * Client socket is a socket that connects to listening server socket. Usually you don't
     * bind it to any specific address/port, and let the OS choose any values it likes. 
     */
    class Socket: public SocketBase
    {
    public:
    
        /**
         * Constructor. 
         * Simply passes the @c netLib parameter to superclass' constructor.
         * @see SocketBase::SocketBase(NetLibrary&)
         */
        Socket(NetLibrary& netLib):
            SocketBase(netLib)
        {}
        
        /**
         * Destructor. Empty.
         */
        ~Socket()
        {}
        
        /**
         * Connects to server socket listening on specified @c SocketAddress.
         * This function makes sense only when used on connection-oriented
         * (stream) socket.
         * @see NetLibSocketConnect()
         */
        Err connect(const SocketAddress& address, Int32 timeout=evtWaitForever);
        
    };
    
}

#endif