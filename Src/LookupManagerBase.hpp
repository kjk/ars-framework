#ifndef __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__
#define __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__

#include <SocketConnection.hpp>
#include <Utility.hpp>

namespace ArsLexis {
    
    class Graphics;
    struct Rectangle;
    
    class LookupProgressReportingSupport;
    
    class LookupProgressReporter {
    public:
    
        virtual void showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const Rectangle& bounds)=0;
        
        virtual ~LookupProgressReporter();
        
    };
    
    class DefaultLookupProgressReporter: public LookupProgressReporter {
    public:
    
        void showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const Rectangle& bounds);
        
        ~DefaultLookupProgressReporter();
        
    };

    class LookupProgressReportingSupport {
        String statusText_;
        uint_t percentProgress_;
        ulong_t bytesProgress_;
        
        typedef std::auto_ptr<LookupProgressReporter> ProgressReporterPtr;
        ProgressReporterPtr progressReporter_;
        
    public:
        
        enum {percentProgressDisabled=(uint_t)-1};
    
        LookupProgressReportingSupport();
        
        const String& statusText() const
        {return statusText_;}
        
        void setStatusText(const String& text)
        {statusText_=text;}
        
        uint_t percentProgress() const
        {return percentProgress_;}
        
        ulong_t bytesProgress() const
        {return bytesProgress_;}
        
       void setPercentProgress(uint_t progress)
       {percentProgress_=progress;}

       void setBytesProgress(ulong_t bytes)
       {bytesProgress_=bytes;}

        void showProgress(Graphics& graphics, const Rectangle& bounds)
        {progressReporter_->showProgress(*this, graphics, bounds);}
        
        virtual ~LookupProgressReportingSupport();
        
        void setProgressReporter(LookupProgressReporter* reporter)
        {progressReporter_.reset(reporter);}
        
    };

    template<uint_t firstLookupEventNumber, class LookupFinishedData>
    class LookupManagerBase: protected LookupProgressReportingSupport, private NonCopyable {
        
        SocketConnectionManager connectionManager_;
        bool lookupInProgress_;
        
    protected:
    
        virtual void handleLookupFinished(const LookupFinishedData& data)
        {}

        using LookupProgressReportingSupport::setBytesProgress;
        using LookupProgressReportingSupport::setPercentProgress;
        using LookupProgressReportingSupport::setStatusText;

    public:
    
        LookupManagerBase(): lookupInProgress_(false) {}
    
        using LookupProgressReportingSupport::showProgress;
    
        enum Event {
            lookupStartedEvent=firstLookupEventNumber,
            lookupProgressEvent,
            lookupFinishedEvent,
            lookupEventCount=lookupFinishedEvent-lookupStartedEvent+1
        };
        
        bool lookupInProgress() const
        {return lookupInProgress_;}
        
        virtual void handleLookupEvent(const EventType& event)
        {
            switch (event.eType)
            {
                case lookupStartedEvent:
                    lookupInProgress_=true;
                    break;
                    
                case lookupFinishedEvent:
                    lookupInProgress_=false;
                    handleLookupFinished(reinterpret_cast<const LookupFinishedData&>(event.data));
                    break;                    
            }
            
        }
        
        SocketConnectionManager& connectionManager()
        {return connectionManager_;}
        
    };

}

#endif
