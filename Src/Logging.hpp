#ifndef __ARSLEXIS_LOGGING_HPP__
#define __ARSLEXIS_LOGGING_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

namespace ArsLexis
{

#pragma mark LogSink
    
    class LogSink
    {
    public:
        virtual ~LogSink() throw()
        {}
        
        virtual void output(const String& str)=0;
    };

#pragma mark -
#pragma mark HostFileLogSink
    
    class HostFileLogSink: public LogSink, private NonCopyable
    {
        HostFILEType* file_;
        
    public:        
        
        explicit HostFileLogSink(const char* fileName) throw();
        
        ~HostFileLogSink() throw();
        
        void output(const String& str);
        
    };
    
#pragma mark -
#pragma mark MemoLogSink
    
    class MemoLogSink: public LogSink, private NonCopyable
    {
        DmOpenRef db_;
        
        void closeDatabase() throw();
        
    public:        
        
        MemoLogSink() throw();
        
        ~MemoLogSink() throw();
        
        void output(const String& str);
        
    };
    
#pragma mark -
#pragma mark DebuggerLogSink
    
    class DebuggerLogSink: public LogSink
    {
    public:        
        
        DebuggerLogSink() throw()
        {}
        
        ~DebuggerLogSink() throw()
        {}
        
        void output(const String& str)
        {
            DbgMessage(str.c_str());
        }
        
    };
    
#pragma mark -
#pragma mark Logger

    class Logger
    {
        const char* context_;
        uint_t contextLength_;
        
    public:
    
        explicit Logger(const char* context) throw():
            context_(context),
            contextLength_(StrLen(context_))
        {}
        
        virtual ~Logger() throw()
        {}
        
        const char* context() const throw()
        {return context_;}
        
        void log(const char* text, uint_t length);
        
        void log(const String& text)
        {log(text.data(), text.length());}
        
        void log(const char* text)
        {log(text, StrLen(text));}
        
        
        class LineAppender
        {
            Logger& logger_;
            String line_;
            bool last_;
            
            LineAppender& operator=(const LineAppender&);
            
        public:
        
            LineAppender(Logger& logger):
                logger_(logger),
                last_(true)
            {}
            
            ~LineAppender()
            {
                if (last_)
                    logger_.log(line_);
            }
            
            LineAppender(LineAppender& prev):
                logger_(prev.logger_),
                last_(true)
            {
                prev.last_=false;
                line_.swap(prev.line_);
            }

            
            LineAppender& operator<<(const char* text)
            {
                line_.append(text);
                return *this;
            }

            LineAppender& operator<<(const String& text)
            {
                line_.append(text);
                return *this;
            }
            
            LineAppender& operator<<(char chr)
            {
                line_.append(1, chr);
                return *this;
            }
            
            LineAppender& operator<<(int i);
            LineAppender& operator<<(unsigned int ui);
            LineAppender& operator<<(short i);
            LineAppender& operator<<(unsigned short ui);
            LineAppender& operator<<(long l);
            LineAppender& operator<<(unsigned long ul);
            
        };

        template<typename T>
        LineAppender operator<<(T val);

    protected:
        
        virtual void logRaw(const String& text)=0;
        
        friend class ChildLogger;
        
    };
    
    template<typename T>
    Logger::LineAppender Logger::operator<<(T val)        
    {
        return LineAppender(*this)<<val;
    }
    
#pragma mark -
#pragma mark RootLogger

    class RootLogger: public Logger, private NonCopyable
    {
        LogSink* sink_;
        
    public:
    
        RootLogger(const char* context, LogSink* sink=0);
        
        LogSink* setSink(LogSink* newSink) throw()
        {
            LogSink* prev=sink_;
            sink_=newSink;
            return prev;
        }
        
        void replaceSink(LogSink* newSink) throw()
        {
            delete setSink(newSink);
        }
        
        LogSink* releaseSink() throw()
        {
            return setSink(0);
        }
        
        ~RootLogger() throw();
        
        static RootLogger* instance() throw();
        
    protected:
        
        void logRaw(const String& text)
        {
            if (sink_)
                sink_->output(text);
        }
    
    };
    
#pragma mark -
#pragma mark ChildLogger

    class ChildLogger: public Logger
    {
        Logger* parent_;
        
    public:
        
        explicit ChildLogger(const char* context) throw():
            Logger(context),
            parent_(RootLogger::instance())
        {}
        
        ChildLogger(const char* context, Logger& parent) throw():
            Logger(context),
            parent_(&parent)
        {}
        
    protected:
    
        void logRaw(const String& text)
        {
            if (parent_)
                parent_->logRaw(text);
        }
        
    };
    
#pragma mark -
#pragma mark FunctionLogger

    class FunctionLogger: public ChildLogger
    {
    public:
    
        FunctionLogger(const char* context, Logger& parent);

        explicit FunctionLogger(const char* context);
        
        ~FunctionLogger() throw();
        
    };
    
    #define FUNCTION_LOG(var) ArsLexis::FunctionLogger var(__FUNCTION__)
    #define FUNCTION_LOG_PARENT(var, parent) ArsLexis::FunctionLogger var(__FUNCTION__, (parent))
    
}

#endif
