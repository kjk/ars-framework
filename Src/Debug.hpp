#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#if __ide_target("Release")
#define NDEBUG
#endif

#ifndef NDEBUG
#define DEBUG
#endif

#include <exception>
#include <cassert>

#ifdef NDEBUG

    #define DEBUG_THROWS(a) throw()
    #define DEBUG_CHECK_LOGIC(condition, text) 

#else

    #define DEBUG_THROWS(a) throw(a)
    #define DEBUG_CHECK_LOGIC(condition, text) if (!(condition)) throw std::logic_error((text))
        
#endif

#define DEBUG_THROWS_LOGIC DEBUG_THROWS(std::logic_error)

class PalmOSError: public std::exception 
{
    Err error_;
public:
    PalmOSError(Err error) throw(): error_(error) {}
    PalmOSError (const PalmOSError& ex): error_(ex.error_) {}
    PalmOSError& operator= (const PalmOSError& ex) throw() {error_=ex.error_; return *this;}
    virtual ~PalmOSError() throw() {}
    virtual const char* what () const throw() {return "PalmOS Error";}
    Err code() const throw() {return error_;}
};

#endif