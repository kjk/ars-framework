/*
  Copyright (C) Krzysztof Kowalczyk
  Owner: Krzysztof Kowalczyk (krzysztofk@pobox.com)

  Definitions of error codes for common code.
*/
#ifndef _ERR_BASE_H_
#define _ERR_BASE_H_

//This breaks noah_palm compilation. if needed for sm, include it in some
//other place
//#include <BaseTypes.hpp>

#if !defined(appErrorClass)

#if defined(_WIN32) || defined(_WIN32_WCE)
#include <winerror.h>

/////////////////////////////////
// The following comes from winerror.h
///////////////////////////////
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//////////////////////////////
// So our error class should look like this:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |1 1|1|0|0 0 0 0 0 0 0 0 0 0 0 0|0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0|
//  +---+-+-+-----------------------+-------------------------------+
//////////////////////////////////

#define appErrorClass 0xe0000000
#define errNone NO_ERROR

#define sysErrParamErr					ERROR_INVALID_DATA
#define memErrNotEnoughSpace		ERROR_NOT_ENOUGH_MEMORY

#define errConnectionFailed appErrorClass+1

// The next lines probably will have to be uncommented for portable SocketConnection to work
// #  define netErrTimeout WSAETIMEDOUT
// #  define netErrWouldBlock WSAEWOULDBLOCK
#else
 #error "Define appErrorClass for your build target."    
#endif  // _WIN32 || _WIN32_WCE
#endif  // !defined(appErrorClass)

#define psErrorClass appErrorClass+0x0100  // PrefsStore

#define sockConnErrorClass appErrorClass+0x0200 // SocketConnection etc.

#define kxmlErrorClass  appErrorClass+0x0300 // KXml2

#define dsErrorClass appErrorClass+0x0400

#define serializerErrorClass appErrorClass+0x0500 // Serializer.hpp

#if !defined(errNone)
# define errNone 0
#endif

#endif
