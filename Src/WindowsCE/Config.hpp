#ifndef GUI_CONFIG_HPP__
#define GUI_CONFIG_HPP__

#if _MSC_VER == 1400 // VC 2005 B2

#ifdef _X86_
    #pragma comment(linker, "/nodefaultlib:libc.lib")
    #pragma comment(linker, "/nodefaultlib:libcd.lib")
    #pragma comment(linker, "/nodefaultlib:oldnames.lib")
#endif

// NOTE - this value is not strongly correlated to the Windows CE OS version being targeted
#ifndef WINVER
#define WINVER _WIN32_WCE
#endif

#endif // _MSC_VER == VC2005

#if _MSC_VER == 1200 // eVC++ 3.0

#ifndef WINVER
#define WINVER 0x400
#endif 

#endif // _MSC_VER

#include <ceconfig.h>

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif

#ifdef SHELL_AYGSHELL
#include <aygshell.h>
#pragma comment(lib, "aygshell.lib")

#define SHELL_MENUBAR

#endif // SHELL_AYGSHELL


// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#if _MSC_VER == 1400 // VC 2005 B2

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#ifndef _DEVICE_RESOLUTION_AWARE
#define _DEVICE_RESOLUTION_AWARE
#endif
#endif

#ifdef _DEVICE_RESOLUTION_AWARE
#include <DeviceResolutionAware.h>
#endif

#include <altcecrt.h>

#ifdef _DLL // /MD
    #if defined(_DEBUG)
        #pragma comment(lib, "msvcrtd.lib")
    #else
        #pragma comment(lib, "msvcrt.lib")
    #endif
#else // /MT
    #if defined(_DEBUG)
        #pragma comment(lib, "libcmtd.lib")
    #else
        #pragma comment(lib, "libcmt.lib")
    #endif
#endif

#if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
    #pragma comment(lib, "ccrtrtti.lib")
    #ifdef _X86_
        #if defined(_DEBUG)
            #pragma comment(lib, "libcmtx86d.lib")
        #else
            #pragma comment(lib, "libcmtx86.lib")
        #endif
    #endif
#endif

#endif // _MSC_VER == VC2005

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
#define SHELL_SIP
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP

#endif // GUI_CONFIG_HPP__