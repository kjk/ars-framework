#ifndef __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__
#define __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__

#include <SocketConnection.hpp>
#include <Utility.hpp>
#include <ExtendedEvent.hpp>

class Graphics;
struct ArsRectangle;

class LookupProgressReportingSupport;

class LookupProgressReporter {
public:

    virtual void showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const ArsRectangle& bounds, bool clearBkg=true)=0;
    
    virtual ~LookupProgressReporter();
    
};

class DefaultLookupProgressReporter: public LookupProgressReporter {
public:

    void showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const ArsRectangle& bounds, bool clearBkg=true);
    
    ~DefaultLookupProgressReporter();
    
};

class LookupProgressReportingSupport 
{
    uint_t percentProgress_;
    ulong_t bytesProgress_;
    
    typedef std::auto_ptr<LookupProgressReporter> ProgressReporterPtr;
    ProgressReporterPtr progressReporter_;
    
public:
    
    enum {percentProgressDisabled=(uint_t)-1};

    LookupProgressReportingSupport();

    const char_t* statusText;
    
    void setStatusText(const char_t *text)
    {statusText=text;}
    
    uint_t percentProgress() const
    {return percentProgress_;}
    
    ulong_t bytesProgress() const
    {return bytesProgress_;}
    
   void setPercentProgress(uint_t progress)
   {percentProgress_=progress;}

   void setBytesProgress(ulong_t bytes)
   {bytesProgress_=bytes;}

    void showProgress(Graphics& graphics, const ArsRectangle& bounds, bool clearBkg=true)
    {
        if (NULL != progressReporter_.get())
            progressReporter_->showProgress(*this, graphics, bounds, clearBkg);
    }
    
    virtual ~LookupProgressReportingSupport();
    
    void setProgressReporter(LookupProgressReporter* reporter)
    {progressReporter_ = ProgressReporterPtr(reporter);}
    
};

template<uint_t firstLookupEventNumber, class LookupFinishedData>
class LookupManagerBase: protected LookupProgressReportingSupport, private NonCopyable {
    
    SocketConnectionManager connectionManager_;
    bool lookupInProgress_;
    
protected:

    virtual void handleLookupFinished(const LookupFinishedData* data)
    {}

    using LookupProgressReportingSupport::setBytesProgress;
    using LookupProgressReportingSupport::setPercentProgress;
    using LookupProgressReportingSupport::setStatusText;

public:

    LookupManagerBase(): lookupInProgress_(false) {}

    using LookupProgressReportingSupport::showProgress;
    using LookupProgressReportingSupport::setProgressReporter;

    enum LookupEvent {
        lookupStartedEvent = firstLookupEventNumber,
        lookupProgressEvent,
        lookupFinishedEvent,
        lookupEventCount = lookupFinishedEvent - lookupStartedEvent + 1
    };
    
    bool lookupInProgress() const
    {return lookupInProgress_;}
    
    virtual void handleLookupEvent(const Event& event)
    {
		ulong_t id = ExtEventGetID(event); 
        switch (id)
        {
            case lookupStartedEvent:
                lookupInProgress_=true;
                break;
                
            case lookupFinishedEvent:
                lookupInProgress_=false;
                handleLookupFinished(static_cast<LookupFinishedData*>(ExtEventGetObject(event)));
                setStatusText(_T(""));
                setPercentProgress(percentProgressDisabled);
                setBytesProgress(0);
                break;                    
        }
    }

    void abortConnections()
    {
        lookupInProgress_=false;
        connectionManager().abortConnections();
    }
    
    SocketConnectionManager& connectionManager()
    {return connectionManager_;}
    
};

#endif
