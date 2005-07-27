#ifndef __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__
#define __ARSLEXIS_LOOKUP_MANAGER_BASE_HPP__

#include <SocketConnection.hpp>
#include <Utility.hpp>
#include <ExtendedEvent.hpp>

class Graphics;
struct Rect;

class LookupProgressReportingSupport;

class LookupProgressReporter {
public:

    virtual void showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const Rect& bounds, bool clearBkg=true)=0;
    
    virtual ~LookupProgressReporter();
    
};

class DefaultLookupProgressReporter: public LookupProgressReporter {
public:

    void showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const Rect& bounds, bool clearBkg=true);
    
    ~DefaultLookupProgressReporter();
    
};

class LookupProgressReportingSupport: private NonCopyable
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

    void showProgress(Graphics& graphics, const Rect& bounds, bool clearBkg = true);

    virtual ~LookupProgressReportingSupport();
    
    void setProgressReporter(LookupProgressReporter* reporter)
    {progressReporter_ = ProgressReporterPtr(reporter);}
    
};

class LookupManagerBase: public LookupProgressReportingSupport  {
    
    SocketConnectionManager connectionManager_;
    bool lookupInProgress_;
	const uint_t lookupStartedEvent_;
    
public:

    LookupManagerBase(uint_t firstEvent);

    bool lookupInProgress() const {return lookupInProgress_;}
    
    virtual void handleLookupEvent(const Event& event);

    void abortConnections();
    
    SocketConnectionManager& connectionManager()
    {return connectionManager_;}
    
};

#endif
