#ifndef __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__
#define __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__

#include <SocketConnection.hpp>
#include <Utility.hpp>

namespace ArsLexis {
    
    class Graphics;
    struct Rectangle;

    class LookupProgressReportingSupport {
        String statusText_;
        uint_t percentProgress_;
        ulong_t bytesProgress_;
        
    public:
        
        static const uint_t percentProgressDisabled;
    
        LookupProgressReportingSupport():
            percentProgress_(percentProgressDisabled),
            bytesProgress_(0)
        {}
        
        const String& statusText() const
        {return statusText_;}
        
        void setStatusText(const String& text)
        {statusText_=text;}
        
        uint_t pecentProgress() const
        {return percentProgress_;}
        
       void setPercentProgress(uint_t progress)
       {percentProgress_=progress;}

       void setBytesProgress(ulong_t bytes)
       {bytesProgress_=bytes;}

        virtual void showProgress(Graphics& graphics, const Rectangle& bounds) const;
        
        virtual ~LookupProgressReportingSupport() {}
        
    };

    template<uint_t firstLookupEventNumber, class LookupFinishedData, class ProgressReportingSupport=LookupProgressReportingSupport>
    class LookupManagerBase: protected ProgressReportingSupport, private NonCopyable {
        
        SocketConnectionManager connectionManager_;
        bool lookupInProgress_;
        
    protected:
    
        virtual void handleLookupFinished(const LookupFinishedData& data)
        {}

        using ProgressReportingSupport::setBytesProgress;
        using ProgressReportingSupport::setPercentProgress;
        using ProgressReportingSupport::setStatusText;

    public:
    
        LookupManagerBase(): lookupInProgress_(false) {}
    
        using ProgressReportingSupport::showProgress;
    
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
