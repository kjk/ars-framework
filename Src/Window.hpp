#ifndef ARSLEXIS_WINDOW_HPP__
#define ARSLEXIS_WINDOW_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

class Rectangle;

class Window: private NonCopyable
{
    WinHandle handle_;
    
public:

    bool valid() const {return NULL != handle_;}

#ifdef NDEBUG
    WinHandle handle() const {return handle_;}
#else
    WinHandle handle() const;
#endif

    Window(): handle_(NULL) {}
    
    virtual ~Window();
    
    Err create(const Rectangle& bounds, FrameType frame, bool modal, bool focusable);
    
    bool modal() const {return WinModal(handle());}
    
    enum EraseOption 
    {
        eraseNot = false,
        eraseWindow = true
    };
    
    void dispose(EraseOption erase = eraseWindow);
    
    void activate() {WinSetActiveWindow(handle());} 
    
};

#endif