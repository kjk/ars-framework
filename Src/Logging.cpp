#include <Logging.hpp>
#include <Application.hpp>
#include <limits>

namespace ArsLexis
{

    void Logger::log(const char* text, uint_t length)
    {
        String full;
        // 8=timestamp; 1=tab; 2=braces; 1=colon; 1=tab; 1=newline; 1=null
        full.reserve(8+1+2+contextLength_+1+1+length+1+1);
        char buffer[9];
        UInt32 timestamp=TimGetTicks();
        StrPrintF(buffer, "%lx", timestamp);
        full.append(buffer, 8).append("\t[", 2).append(context_, contextLength_).append("]:\t", 3).append(text, length).append(1, '\n');
        logRaw(full);
    }
    
    Logger::LineAppender& Logger::LineAppender::operator<<(unsigned short ui)
    {
        char buffer[26];
        Int16 len=StrPrintF(buffer, "%hu (=0x%hx)", ui, ui);
        if (len>0)
            line_.append(buffer, len);
        return *this;
    }

    Logger::LineAppender& Logger::LineAppender::operator<<(short i)
    {
        char buffer[26];
        Int16 len=StrPrintF(buffer, "%hd (=0x%hx)", i, i);
        if (len>0)
            line_.append(buffer, len);
        return *this;
    }

    RootLogger::RootLogger(const char* context, LogSink* sink):
        Logger(context),
        sink_(sink)
    {
        Err error=FtrSet(Application::creator(), Application::featureRootLoggerPointer, reinterpret_cast<UInt32>(this));
        if (error)
            (*this)<<"Unable to set RootLogger instance pointer, error: "<<error;
    }
        
    RootLogger::~RootLogger() throw()
    {
        FtrUnregister(Application::creator(), Application::featureRootLoggerPointer);
        delete sink_;
    }

    RootLogger* RootLogger::instance() throw()
    {
        RootLogger* logger=0;
        Err error=FtrGet(Application::creator(), Application::featureRootLoggerPointer, reinterpret_cast<UInt32*>(&logger));
        if (error)
            assert(false);
        return logger;
    }

    FunctionLogger::FunctionLogger(const char* context, Logger& parent):
        ChildLogger(context, parent)
    {
        log(">>> Enter");
    }
        

    FunctionLogger::FunctionLogger(const char* context):
        ChildLogger(context)
    {
        log(">>> Enter");
    }
    
    FunctionLogger::~FunctionLogger() throw()
    {
//        try {
            log("<<< Exit");
//        }
//        catch (...) {
//        }
    }

#pragma mark -
#pragma mark HostFileLogSink    

    HostFileLogSink::HostFileLogSink(const char* fileName) throw():
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
#pragma mark MemoLogSink

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
        const char* text=str.c_str();
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

}
