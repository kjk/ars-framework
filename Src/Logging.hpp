#ifndef __ARSLEXIS_LOGGING_HPP__
#define __ARSLEXIS_LOGGING_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>
#include <list>

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
    
        explicit Logger(const char* context) throw():
            context_(context),
            contextLength_(StrLen(context_)),
            threshold_(logLevelDefault)
        {}
        
        uint_t threshold() const
        {return threshold_;}
        
        void setThreshold(uint_t threshold)
        {threshold_=threshold;}
        
        virtual ~Logger() throw()
        {}
        
        const char* context() const throw()
        {return context_;}
        
        void log(const char* text, uint_t length, uint_t level=logLevelDefault);
        
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
            
            LineAppender(Logger& logger, uint_t level, bool doLog):
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
            
            LineAppender& operator<<(const char* text)
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
            
            LineAppender& operator<<(char chr)
            {
                if (log_)
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
    
    template<typename T>
    Logger::LineAppender Logger::operator<<(T val)        
    {return LineAppender(*this, logLevelDefault<=threshold_, logLevelDefault)<<val;}

    template<>
    Logger::LineAppender Logger::operator<< <const char*> (const char* val);
    
#pragma mark -
#pragma mark RootLogger

    class RootLogger: public Logger, private NonCopyable
    {
        typedef std::pair<LogSink*, uint_t> SinkWithThreshold;
        typedef std::list<SinkWithThreshold> Sinks_t;
        Sinks_t sinks_;
   
        static void deleteSink(SinkWithThreshold& s) 
        {delete s.first;}            
     
    public:
    
        RootLogger(const char* context, LogSink* sink=0, uint_t sinkThreshold=logLevelDefault);
        
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
        
        explicit ChildLogger(const char* context) throw():
            Logger(context),
            parent_(RootLogger::instance())
        {}
        
        ChildLogger(const char* context, Logger& parent) throw():
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
    
        FunctionLogger(const char* context, Logger& parent);

        explicit FunctionLogger(const char* context);
        
        ~FunctionLogger() throw();
        
    };
    
    #define FUNCTION_LOG(var) ArsLexis::FunctionLogger var(__FUNCTION__)
    #define FUNCTION_LOG_PARENT(var, parent) ArsLexis::FunctionLogger var(__FUNCTION__, (parent))
    
}

#endif
