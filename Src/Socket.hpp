/**
 * @file Socket.hpp
 * C++ wrappers for PalmOS sockets implementation.
 * 
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_SOCKET_HPP__
#define __ARSLEXIS_SOCKET_HPP__

#include <Debug.hpp>
#include <Logging.hpp>
#include <NativeSocks.hpp>

namespace ArsLexis
{

    class NetLibrary;
    
    class SocketAddress;

    /**
     * Base class for @c Socket and @c ServerSocket.
     * Implements functionality common to both socket types. Can't be instantiated stand-alone,
     * as it wouldn't make much sense anyway.
     */
    class SocketBase: private NonCopyable
    {

        mutable ChildLogger log_;

    protected:
    
        Logger& log() const
        {return log_;}
    
        /**
         * Stores reference to currently loaded NetLib.
         */
        NetLibrary& netLib_;

        /**
         * Handle to underlying PalmOS socket.
         */
         NativeSocket_t socket_;

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
        status_t open(NativeSockAddrFamily_t domain = SocketAddrINET_c,
            NativeSocketType_t type= SocketTypeStream_c,
            short protocol=0, long timeout=evtWaitForever);

        status_t shutdown(short direction, long timeout=evtWaitForever);
        
        status_t send(ushort_t& sent, const void* buffer, ushort_t bufferLength, long timeout=evtWaitForever, ushort_t flags=0, const SocketAddress* address=0);
        
        status_t receive(ushort_t& received, void* buffer, ushort_t bufferLength, long timeout=evtWaitForever, ushort_t flags=0);
        
        operator  NativeSocket_t() const
        {return socket_;}

        status_t setNonBlocking(bool value=true);
        
        status_t setOption(ushort_t level, ushort_t option, void* optionValue, ushort_t valueLength, long timeout=evtWaitForever);
        
        status_t getOption(ushort_t level, ushort_t option, void* optionValue, ushort_t& valueLength, long timeout=evtWaitForever) const;
        
        status_t setLinger(const CommonSocketLinger_t& linger);
        
        //status_t getLinger(CommonSocketLinger_t& linger) const;
                
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
        status_t connect(const SocketAddress& address, long timeout=evtWaitForever);
        
    };
    
    class SocketSelector: private NonCopyable
    {
        enum {eventTypesCount_=3};
        
        NetLibrary& netLib_;
        NativeFDSet_t inputFDs_[eventTypesCount_];
        NativeFDSet_t outputFDs_[eventTypesCount_];
        ushort_t width_;
        ushort_t eventsCount_;
        
        void recalculateWidth();
        
    public:
    
        enum EventType
        {
            eventRead,
            eventWrite,
            eventException
        };
        
        SocketSelector(NetLibrary& netLib, bool catchStandardEvents=true);
        
        ~SocketSelector()
        {}
        
        void registerSocket(const SocketBase& socket, EventType event)
        {
             NativeSocket_t ref=socket;
            netFDSet(ref, &inputFDs_[event]);
            recalculateWidth();
        }
        
        void unregisterSocket(const SocketBase& socket, EventType event)
        {
             NativeSocket_t ref=socket;
            netFDClr(ref, &inputFDs_[event]);
            recalculateWidth();
        }
        
        status_t select(long timeout=evtWaitForever);
        
        bool checkSocketEvent(const SocketBase& socket, EventType event) const
        {
             NativeSocket_t ref=socket;
            return netFDIsSet(ref, &outputFDs_[event]);
        }
        
        bool isRegistered(const SocketBase& socket, EventType event) const
        {
             NativeSocket_t ref=socket;
            return netFDIsSet(ref, &inputFDs_[event]);
        }
        
        /*bool checkStandardEvent() const
        {
            return netFDIsSet(sysFileDescStdIn, &outputFDs_[eventRead]);
        }*/

        ushort_t eventsCount() const
        {return eventsCount_;}
        
        bool active() const
        {return inputFDs_[eventRead] || inputFDs_[eventWrite] || inputFDs_[eventException];}
        
    };
    
}

#endif
