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
static const UINT extEvent = WM_USER;
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

/*

void *    createExtendedEventText(ulong_t eventId, const ArsLexis::char_t* txt, ulong_t length);
void      sendExtendedEvent(void* eventData, const Point* point = NULL);
ulong_t   getExtendedEventId(void *eventData);
ulong_t   getExtendedEventId(EventType *event);
ulong_t   getExtendedEventMagicNumber(void *eventData);
ulong_t   getExtendedEventType(void *eventData);
ArsLexis::char_t* getTextEventDataCopy(EventType *event, ulong_t& length);
const ArsLexis::char_t* getTextEventData(EventType* event);
ulong_t getTextEventDataLength(EventType* event);

void freeExtendedEvent(EventType *event);

void sendTextEvent(ulong_t eventId, const ArsLexis::char_t* txt,  const Point* point = NULL);

void sendTextNEvent(ulong_t eventId, const ArsLexis::char_t* txt, ulong_t length, const Point* point = NULL);
 */

void ExtEventFree(Event& event);
ExtEventType ExtEventGetType(const Event& event);
ulong_t ExtEventGetID(const Event& event);
const Point& ExtEventGetPoint(const Event& event);
const char_t* ExtEventGetText(const Event& event, ulong_t* textLen = NULL);
const char* ExtEventGetNarrowText(const Event& event, ulong_t* textLen = NULL);
const void* ExtEventGetData(const Event& event, ulong_t& dataLen);
const ExtEventObject* ExtEventGetObject(const Event& event);

status_t ExtEventSendEmpty(ulong_t id, const Point* p = NULL);
status_t ExtEventSendText(ulong_t id, const char_t* text, long len = -1, const Point* p = NULL);
status_t ExtEventSendNarrow(ulong_t id, const char* text, long len = -1, const Point* p = NULL);
status_t ExtEventSendBlob(ulong_t id, const void* blob, ulong_t len, const Point* p = NULL);
status_t ExtEventSendObject(ulong_t id, ExtEventObject* object, const Point* p = NULL);

#ifndef NDEBUG
void test_ExtEventSend();
void test_ExtEventReceive(Event& event);
#endif

#endif
