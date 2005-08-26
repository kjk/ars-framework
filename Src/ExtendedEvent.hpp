#ifndef ARSLEXIS_EXTENDED_EVENT_HPP__
#define ARSLEXIS_EXTENDED_EVENT_HPP__

#include <Debug.hpp>

struct Point;

#ifdef _PALM_OS
#include <Application.hpp>
static const eventsEnum extEvent = eventsEnum(Application::appExtendedEvent);
typedef EventType Event;
#endif

#ifdef _WIN32
typedef LPARAM Event;
static const UINT extEvent = WM_APP + 0x1000;
#endif

enum {extEventMagicNumber = ulong_t(901107055L)};
enum ExtEventType {
	extEventTypeEmpty,
	extEventTypeText,
	extEventTypeNarrowText,
	extEventTypeBlob,
	extEventTypeObject
};

struct ExtEventObject {
	virtual ~ExtEventObject();
};

#ifdef _WIN32
HWND ExtEventSetWindow(HWND wnd);
HWND ExtEventGetWindow();

class ExtEventHelper {
    HWND prevEventWindow_;
public:
    ExtEventHelper(): 
        prevEventWindow_(HWND(-1)) 
    {
    }
    
    void start(HWND thisWindow) 
    {
        prevEventWindow_ = ExtEventSetWindow(thisWindow);
    }
   
    void stop() 
    {
        if (HWND(-1) != prevEventWindow_) 
            ExtEventSetWindow(prevEventWindow_);
    }  
     
    ~ExtEventHelper() 
    {
        stop(); 
    }
    
};
   
#define EXT_EVENT_WINDOW_PARAM(name) , HWND name = NULL

#else
#define EXT_EVENT_WINDOW_PARAM(name) 
#endif

void ExtEventFree(Event& event);
void ExtEventRepost(Event& event EXT_EVENT_WINDOW_PARAM(wnd));
ExtEventType ExtEventGetType(const Event& event);
ulong_t ExtEventGetID(const Event& event);
const Point& ExtEventGetPoint(const Event& event);
const char_t* ExtEventGetText(const Event& event, ulong_t* textLen = NULL);
const char* ExtEventGetNarrowText(const Event& event, ulong_t* textLen = NULL);
const void* ExtEventGetData(const Event& event, ulong_t& dataLen);
const ExtEventObject* ExtEventGetObject(const Event& event);

status_t ExtEventSendEmpty(ulong_t id, const Point* p = NULL EXT_EVENT_WINDOW_PARAM(wnd));
status_t ExtEventSendText(ulong_t id, const char_t* text, long len = -1, const Point* p = NULL EXT_EVENT_WINDOW_PARAM(wnd));
status_t ExtEventSendNarrow(ulong_t id, const char* text, long len = -1, const Point* p = NULL EXT_EVENT_WINDOW_PARAM(wnd));
status_t ExtEventSendBlob(ulong_t id, const void* blob, ulong_t len, const Point* p = NULL EXT_EVENT_WINDOW_PARAM(wnd));
status_t ExtEventSendObject(ulong_t id, ExtEventObject* object, const Point* p = NULL EXT_EVENT_WINDOW_PARAM(wnd));

#undef EXT_EVENT_WINDOW_PARAM

#ifndef NDEBUG
void test_ExtEventSend();
void test_ExtEventReceive(Event& event);
#endif

#endif
