#ifndef __ARSLEXIS_LOGGING_HPP__
#define __ARSLEXIS_LOGGING_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>
#include <list>

#ifdef _MSC_VER
# pragma warning(disable: 4068)
# pragma warning(disable: 4290)  // warning C++ Exception Specification ignored
#endif

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

#ifdef _PALM_OS
    
    class HostFileLogSink: public LogSink, private NonCopyable
    {
        HostFILEType* file_;
        
    public:        
        
        explicit HostFileLogSink(const char_t* fileName) throw();
        
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

#endif    
    
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
#if defined(_PALM_OS)
            DbgMessage(str.c_str());
#elif defined(_WIN32_WCE)
            OutputDebugString(str.c_str());
#else
# error "Define version of DebuggerLogSink appropriate for your system."
#endif            
        }
        
    };
    
#pragma mark -
#pragma mark Logger

    class Logger
    {
        const char_t* context_;
        uint_t contextLength_;
        uint_t threshold_;
        
    public:
    
        enum Level 
        {
            logAlways,
            logCritical=10,
            logError=20,
            logDebug=30,
            logWarning=40,
            logInfo=50,
            logEverything=(uint_t)-1,
            logLevelDefault=logDebug
        };
    
        explicit Logger(const char_t* context) throw():
            context_(context),
            contextLength_(tstrlen(context_)),
            threshold_(logLevelDefault)
        {}
        
        uint_t threshold() const
        {return threshold_;}
        
        void setThreshold(uint_t threshold)
        {threshold_=threshold;}
        
        virtual ~Logger() throw()
        {}
        
        const char_t* context() const throw()
        {return context_;}
        
        void log(const char_t* text, uint_t length, uint_t level=logLevelDefault);
        
        void log(const String& text, uint_t level=logLevelDefault)
        {log(text.data(), text.length(), level);}
        
        class LineAppender
        {
            Logger& logger_;
            String line_;
            uint_t level_;
            uint_t last_:1;
            uint_t log_:1;
            
            LineAppender& operator=(const LineAppender&);
            
            LineAppender(Logger& logger, bool doLog, uint_t level):
                logger_(logger),
                level_(level),
                last_(true),
                log_(doLog)
            {}
            
            LineAppender(LineAppender& prev):
                logger_(prev.logger_),
                last_(true),
                log_(prev.log_)
            {
                prev.last_=false;
                if (log_)
                    line_.swap(prev.line_);
            }
            
            friend class Logger;

        public:
        
            ~LineAppender()
            {
                if (last_ && log_)
                    logger_.log(line_, level_);
            }
            
            LineAppender& operator<<(const char_t* text)
            {
                if (log_)
                    line_.append(text);
                return *this;
            }

            LineAppender& operator<<(const String& text)
            {
                if (log_)
                    line_.append(text);
                return *this;
            }
            
            LineAppender& operator<<(char_t chr)
            {
                if (log_)
                    line_.append(1, chr);
                return *this;
            }
            
            LineAppender& operator<<(int i);
            LineAppender& operator<<(unsigned int ui);
            LineAppender& operator<<(short i);
#ifndef _WIN32_WCE
            LineAppender& operator<<(unsigned short ui);
#endif // _WIN32_WCE
            LineAppender& operator<<(long l);
            LineAppender& operator<<(unsigned long ul);
            
        };

        template<class T>
        LineAppender operator<<(T val);
        
        LineAppender operator()(uint_t level);

        LineAppender critical();

        LineAppender error();
        
        LineAppender debug();

        LineAppender warning();

        LineAppender info();

    protected:
        
        virtual void logRaw(const String& text, uint_t level)=0;
        
        friend class ChildLogger;
        
    };
    
    template<class T>
    Logger::LineAppender Logger::operator<<(T val)        
    {return LineAppender(*this, logLevelDefault<=threshold_, logLevelDefault)<<val;}

#ifndef _MSC_VER
    template<>
    Logger::LineAppender Logger::operator<< < const char_t* > (const char_t* val);
#endif // _MSC_VER
    
#pragma mark -
#pragma mark RootLogger

    class RootLogger: public Logger, private NonCopyable
    {
        struct SinkWithThreshold {
            LogSink* sink;
            uint_t threshold;
            SinkWithThreshold(LogSink* s, uint_t thr): sink(s), threshold(thr) {}
            ~SinkWithThreshold() {delete sink;}
        };

        typedef std::list<SinkWithThreshold*> Sinks_t;
        Sinks_t sinks_;
   
    public:
    
        RootLogger(const char_t* context, LogSink* sink=0, uint_t sinkThreshold=logLevelDefault);
        
        void addSink(LogSink* newSink, uint_t threshold=logLevelDefault) throw();
        
        ~RootLogger() throw();
        
        static RootLogger* instance() throw();
        
    protected:
        
        void logRaw(const String& text, uint_t level);
    
    };
    
#pragma mark -
#pragma mark ChildLogger

    class ChildLogger: public Logger
    {
        Logger* parent_;
        
    public:
        
        explicit ChildLogger(const char_t* context) throw():
            Logger(context),
            parent_(RootLogger::instance())
        {}
        
        ChildLogger(const char_t* context, Logger& parent) throw():
            Logger(context),
            parent_(&parent)
        {}
        
    protected:
    
        void logRaw(const String& text, uint_t level)
        {
            if (parent_)
                parent_->logRaw(text, level);
        }
        
    };
    
#pragma mark -
#pragma mark FunctionLogger

    class FunctionLogger: public ChildLogger
    {
    public:
    
        FunctionLogger(const char_t* context, Logger& parent);

        explicit FunctionLogger(const char_t* context);
        
        ~FunctionLogger() throw();
        
    };
    
    #define FUNCTION_LOG(var) ArsLexis::FunctionLogger var(__FUNCTION__)
    #define FUNCTION_LOG_PARENT(var, parent) ArsLexis::FunctionLogger var(__FUNCTION__, (parent))
    
}

#endif
