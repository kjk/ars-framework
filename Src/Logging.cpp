#include <Logging.hpp>
#include <algorithm>

#ifdef _PALM_OS
#include <Application.hpp>
#endif // _PALM_OS

#ifdef __MWERKS__
# pragma far_code
# pragma inline_bottom_up on
#endif


namespace ArsLexis
{

    void Logger::log(const char_t* text, uint_t length, uint_t level)
    {
        String full;
        // 8=timestamp; 1=tab; 2=braces; 1=colon; 1=tab; 1=newline; 1=null
        full.reserve(8+1+2+contextLength_+1+1+length+1+1);
        char_t buffer[9];
        tick_t timestamp=ticks();
        tprintf(buffer, _T("%08lx"), timestamp);
        full.append(buffer, 8).append(_T("\t["), 2).append(context_, contextLength_).append(_T("]:\t"), 3).append(text, length).append(1, _T('\n'));
        logRaw(full, level);
    }

#ifndef _WIN32_WCE    
    Logger::LineAppender& Logger::LineAppender::operator<<(unsigned short ui)
    {
        if (log_)
        {
            char_t buffer[26];
            int len=tprintf(buffer, _T("%hu (0x%04hx)"), ui, ui);
            if (len>0)
                line_.append(buffer, len);
        }                
        return *this;
    }

#endif // )WIN32_WCE

    Logger::LineAppender& Logger::LineAppender::operator<<(unsigned long l)
    {
        if (log_)
        {
            char_t buffer[26];
            int len=tprintf(buffer, _T("%lu (0x%08lx)"), l, l);
            if (len>0)
                line_.append(buffer, len);
        }                
        return *this;
    }

    Logger::LineAppender& Logger::LineAppender::operator<<(long l)
    {
        if (log_)
        {
            char_t buffer[26];
            int len=tprintf(buffer, _T("%ld (0x%08lx)"), l, l);
            if (len>0)
                line_.append(buffer, len);
        }                
        return *this;
    }

    Logger::LineAppender& Logger::LineAppender::operator<<(short i)
    {
        if (log_)
        {
            char_t buffer[26];
            int len=tprintf(buffer, _T("%hd (0x%04hx)"), i, i);
            if (len>0)
                line_.append(buffer, len);
        }                
        return *this;
    }

    Logger::LineAppender& Logger::LineAppender::operator<<(int i)
    {
#if defined(_PALM_OS)    
        return *this<<static_cast<short>(i);
#else
        return *this<<static_cast<long>(i);
#endif        
    }

    Logger::LineAppender& Logger::LineAppender::operator<<(unsigned int i)
    {
#if defined(_PALM_OS)    
        return *this<<static_cast<unsigned short>(i);
#else
        return *this<<static_cast<unsigned long>(i);
#endif        
    }

    Logger::LineAppender Logger::operator()(uint_t level)
    {
        return LineAppender(*this, level<=threshold_, level);
    }
    
    Logger::LineAppender Logger::critical()
    {
        return LineAppender(*this, logCritical<=threshold_, logCritical);
    }

    Logger::LineAppender Logger::error()
    {
        return LineAppender(*this, logError<=threshold_, logError);
    }
    
    Logger::LineAppender Logger::debug()
    {
        return LineAppender(*this, logDebug<=threshold_, logDebug);
    }

    Logger::LineAppender Logger::warning()
    {
        return LineAppender(*this, logWarning<=threshold_, logWarning);
    }

    Logger::LineAppender Logger::info()
    {
        return LineAppender(*this, logInfo<=threshold_, logInfo);
    }

#ifndef _MSC_VER
    // This explicit specialization doesn't differ in any way from the one that would be generated.
    // But it's here because it reduces size of code significantly.
    template<>
    Logger::LineAppender Logger::operator<< <const char_t*> (const char_t* val)
    {
        return LineAppender(*this, logLevelDefault<=threshold_, logLevelDefault)<<val;
    }
#endif  // _MSC_VER

#pragma mark -

#ifndef _PALM_OS
		
	namespace {
		RootLogger* rootLoggerInstance=0;
	}

#endif // _PALM_OS

    RootLogger::RootLogger(const char_t* context, LogSink* sink, uint_t sinkThreshold):
        Logger(context)
    {
        if (sink)
            addSink(sink, sinkThreshold);
#ifdef _PALM_OS
        Err err=FtrSet(Application::creator(), Application::featureRootLoggerPointer, reinterpret_cast<UInt32>(this));
        if (err)
            error()<<_T("Unable to set RootLogger instance pointer, error: ")<<err;
#else
        assert(rootLoggerInstance==0);
        rootLoggerInstance=this;
#endif // _PALM_OS
    }

    RootLogger::~RootLogger() throw()
    {
#ifdef _PALM_OS
        FtrUnregister(Application::creator(), Application::featureRootLoggerPointer);
#else
        rootLoggerInstance=0;
#endif // _PALM_OS
        std::for_each(sinks_.begin(), sinks_.end(), ObjectDeleter<SinkWithThreshold>());        
    }

    RootLogger* RootLogger::instance() throw()
    {
#ifdef _PALM_OS
        RootLogger* logger=0;
        Err error=FtrGet(Application::creator(), Application::featureRootLoggerPointer, reinterpret_cast<UInt32*>(&logger));
        if (error)
            assert(false);
        return logger;
#else
        return rootLoggerInstance;
#endif // _PALM_OS
    }
    
    void RootLogger::addSink(LogSink* newSink, uint_t threshold) throw()
    {
        assert(newSink);
        sinks_.push_back(new SinkWithThreshold(newSink, threshold));
    }

    void RootLogger::logRaw(const String& text, uint_t level)
    {
        Sinks_t::iterator end=sinks_.end();
        for (Sinks_t::iterator it=sinks_.begin(); it!=end; ++it)
            if ((*it)->threshold>=level)
                (*it)->sink->output(text);
    }

#pragma mark -

    FunctionLogger::FunctionLogger(const char_t* context, Logger& parent):
        ChildLogger(context, parent)
    {
        log(_T(">>> Enter"));
    }
        

    FunctionLogger::FunctionLogger(const char_t* context):
        ChildLogger(context)
    {
        log(_T(">>> Enter"));
    }
    
    FunctionLogger::~FunctionLogger() throw()
    {
        log(_T("<<< Exit"));
    }

#pragma mark -

#ifdef _PALM_OS

    HostFileLogSink::HostFileLogSink(const char_t* fileName) throw():
        file_(HostFOpen(fileName, "w"))
    {}
    
    HostFileLogSink::~HostFileLogSink() throw()
    {
        if (file_)
        {
            HostFFlush(file_);
            HostFClose(file_);
        }
    }
    
    void HostFileLogSink::output(const String& str)
    {
        if (file_)
        {
            HostFPutS(str.c_str(), file_);
            HostFFlush(file_);
        }
    }

#pragma mark -

    MemoLogSink::MemoLogSink() throw():
        db_(NULL)
    {
        db_=DmOpenDatabaseByTypeCreator('DATA', 'memo', dmModeReadWrite);
        if (NULL==db_)
        {
            Err error=DmGetLastErr();
        }
    }
    
    void MemoLogSink::closeDatabase() throw()
    {
        DmCloseDatabase(db_);
        db_=NULL;
    }
    
    MemoLogSink::~MemoLogSink() throw()
    {
        if (db_)
            closeDatabase();
    }
    
    void MemoLogSink::output(const String& str)
    {
        if (!db_)
            return;
        UInt16 index=dmMaxRecordIndex;
        const char_t* text=str.c_str();
        UInt16 len=StrLen(text)+1;
        MemHandle handle=DmNewRecord(db_, &index, len);
        if (!handle)
        {
            closeDatabase();
            return;
        }
        void* data=MemHandleLock(handle);
        if (data)
        {
            DmWrite(data, 0, text, len);
            MemHandleUnlock(handle);
        }
        DmReleaseRecord(db_, index, true);
    }
    
#endif // _PALM_OS

}

#if defined(_PALM_OS)

void ArsLexis::cleanAllocationLogging()
{
    HostFILEType* file=0;
    UInt32 creatorId=Application::creator();
    Err error=FtrGet(creatorId, Application::featureAllocationLoggerPointer, reinterpret_cast<UInt32*>(&file));
    if (errNone==error && 0!=file)
        HostFClose(file);
    FtrUnregister(creatorId, Application::featureAllocationLoggerPointer);
}

#if !defined(ALLOCATION_LOG_PATH)
# error "Define ALLOCATION_LOG_PATH in yer Target.h"
#endif

void ArsLexis::logAllocation(void* ptr, size_t size, bool free, const char* fileName, int line)
{
#if defined(ARSLEXIS_DEBUG_MEMORY_ALLOCATION)
    HostFILEType* file=0;
    UInt32 creatorId=Application::creator();
    Err error=FtrGet(creatorId, Application::featureAllocationLoggerPointer, reinterpret_cast<UInt32*>(&file));
    if (errNone!=error)
    {
        file=HostFOpen(ALLOCATION_LOG_PATH, "w");
        if (file)
            FtrSet(creatorId, Application::featureAllocationLoggerPointer, reinterpret_cast<UInt32>(file));
    }
    if (0!=file)
    {
        if (free)
            HostFPrintF(file, "-\t0x%08lx\n", ptr);
        else
            HostFPrintF(file, "+\t0x%08lx\t%ld\t%s: %hd\n", ptr, size, 0==fileName?"(Unknown)":fileName, line);
        HostFFlush(file);
    }
#endif    
}

#endif // _PALMOS
