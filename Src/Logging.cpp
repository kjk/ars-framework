#include "Logging.hpp"
#include "Application.hpp"

namespace ArsLexis
{

    void Logger::log(const String& text)
    {
        String full;
        // 8=timestamp; 1=tab; 2=braces; 1=colon; 1=tab; 1=newline; 1=null
        full.reserve(8+1+2+contextLength_+1+1+text.length()+1+1);
        char buffer[9];
        UInt32 timestamp=TimGetTicks();
        StrPrintF(buffer, "%lx", timestamp);
        full.append(buffer, 8).append("\t[", 2).append(context_, contextLength_).append("]:\t", 3).append(text).append(1, '\n');
        logRaw(full);
    }

    RootLogger::RootLogger(const char* context, const char* fileName):
        Logger(context),
        file_(HostFOpen(fileName, "w"))
    {
    }

    RootLogger::~RootLogger()
    {
        if (file_)
            HostFClose(file_);
    }

    RootLogger* RootLogger::instance()
    {
        RootLogger* logger=0;
        
        
        return logger;
    }

    void RootLogger::logRaw(const String& text)
    {
        if (file_)
        {
            HostFPutS(text.c_str(), file_);
            HostFFlush(file_);
        }
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
    
    FunctionLogger::~FunctionLogger()
    {
        log("<<< Exit");
    }

}