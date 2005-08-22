#ifndef ARSLEXIS_LOCK_HPP__
#define ARSLEXIS_LOCK_HPP__

#include <Debug.hpp>
#include <Utility.hpp>

class DummyLock: private NonCopyable {

public:

    DummyLock() {}
    ~DummyLock() {} 

    void acquire() {}
    void release() {} 

};

template<class L> class Guard: private NonCopyable {
    L& lock_;
    bool locked_; 

public:
    Guard(L& l, bool lock = true): lock_(l) {if (lock) acquire();}
    void acquire() {lock_.acquire(); locked_ = true;}
    void release() {if (locked_) {lock_.release(); locked_ = false;}}  
    ~Guard() {release();}
};

template<> class Guard<DummyLock>: private NonCopyable {
public:
    Guard(DummyLock&, bool) {}
    void acquire() {}
    void release() {}
    ~Guard() {}   
};

#ifdef _WIN32
class CriticalSection: private NonCopyable {
    CRITICAL_SECTION section_;
public:

    CriticalSection();
    ~CriticalSection();
    void acquire();
    void release();
};

typedef CriticalSection Lock;
#endif // _WIN32

#ifdef _PALM_OS
typedef DummyLock Lock;
#endif // _PALM_OS

typedef Guard<Lock> LockGuard;
#endif