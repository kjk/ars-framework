#include <ExtendedEvent.hpp>
#include <Text.hpp>
#include <Geometry.hpp>

#ifdef __MWERKS__
using std::memset;
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4200)
#endif

struct ExtEventData {
	ulong_t magic;
	ulong_t id;
	union {
		ExtEventType type;
		ulong_t fill_;
	};
	Point point;
	ulong_t postCount;
}; 

ExtEventObject::~ExtEventObject() {} 

static ExtEventData*& ExtEventExtractDataPtr(Event& event)
{
#ifdef _WIN32
	ExtEventData*& data = (ExtEventData*&)event;
#endif
#ifdef _PALM_OS
	ExtEventData*& data = (ExtEventData*&)event.data;
#endif	
	assert(NULL != data);
	assert(extEventMagicNumber == data->magic);
	return data;
}

static const ExtEventData& ExtEventExtractData(const Event& event)
{
	ExtEventData*& data = ExtEventExtractDataPtr((Event&)event);
	return *data;
}

static const void* ExtEventExtractContents(const Event& event)
{
	char* p = (char*)&ExtEventExtractData(event);
	p += sizeof(ExtEventData);
	return p;
}

static void ExtEventSend(void* data, void* wnd)
{
#ifdef _WIN32
	PostMessage((HWND)wnd, extEvent, 0, LPARAM(data));
#endif

#ifdef _PALM_OS
    EventType event;
    MemSet(&event, sizeof(event), 0);
    event.eType = extEvent;
	void** p = (void**)&event.data;
	*p = data;
	EvtAddEventToQueue(&event);
#endif
}

void ExtEventRepost(Event& event, void* wnd)
{
	ExtEventData*& data = ExtEventExtractDataPtr(event);
	assert(data != NULL);
	++data->postCount;
	ExtEventSend(data, wnd);
}

void ExtEventFree(Event& event)
{
	ExtEventData*& data = ExtEventExtractDataPtr(event);
	assert(data != NULL);
	if (0 == --data->postCount)
	{
	    if (extEventTypeObject == data->type)
	    {
		    ExtEventObject* object = const_cast<ExtEventObject*>(ExtEventGetObject(event));
		    delete object;
	    }
	    free(data);
    }
	data = NULL;
}

static char* ExtEventCreate(ulong_t id, ExtEventType type, ulong_t dataSize, const Point* p)
{
	ulong_t sz = sizeof(ExtEventData) + dataSize;
	ExtEventData* data = (ExtEventData*)malloc(sz);
	if (NULL == data)
		return NULL;
	
	memset(data, 0, sz);
	data->magic = extEventMagicNumber;
	data->id = id;
	data->type = type;
	data->postCount = 1;
	if (NULL != p)
		data->point = *p;
	
	return (char*)data;
}

ExtEventType ExtEventGetType(const Event& event)
{
	return ExtEventExtractData(event).type;
}

ulong_t ExtEventGetID(const Event& event)
{
	return ExtEventExtractData(event).id;
}

const Point& ExtEventGetPoint(const Event& event)
{
	return ExtEventExtractData(event).point;
}

struct ExtEventTextContents {
	ulong_t length;
	char_t data[];
};

const char_t* ExtEventGetText(const Event& event, ulong_t* textLen)
{
	assert(extEventTypeText == ExtEventGetType(event));
	const ExtEventTextContents* contents = (const ExtEventTextContents*)ExtEventExtractContents(event);
	if (NULL != textLen)
		*textLen = contents->length;
	return contents->data;
}

struct ExtEventNarrowContents {
	ulong_t length;
	char data[];
};

const char* ExtEventGetNarrowText(const Event& event, ulong_t* textLen)
{
	assert(extEventTypeNarrowText == ExtEventGetType(event));
	const ExtEventNarrowContents* contents = (const ExtEventNarrowContents*)ExtEventExtractContents(event);
	if (NULL != textLen)
		*textLen = contents->length;
	return contents->data;
}

const void* ExtEventGetData(const Event& event, ulong_t& dataLen)
{
	assert(extEventTypeBlob == ExtEventGetType(event));
	const ExtEventNarrowContents* contents = (const ExtEventNarrowContents*)ExtEventExtractContents(event);
	dataLen = contents->length;
	return contents->data;
}

struct ExtEventObjectContents {
	ExtEventObject* object;
};

const ExtEventObject* ExtEventGetObject(const Event& event)
{
	assert(extEventTypeObject == ExtEventGetType(event));
	const ExtEventObjectContents* contents = (const ExtEventObjectContents*)ExtEventExtractContents(event);
	return contents->object;
}

status_t ExtEventSendEmpty(ulong_t id, const Point* p, void* wnd)
{
	void* data = ExtEventCreate(id, extEventTypeEmpty, 0, p);
	if (NULL == data)
		return memErrNotEnoughSpace;
	ExtEventSend(data, wnd);
	return errNone;
}

status_t ExtEventSendText(ulong_t id, const char_t* text, long len, const Point* p, void* wnd)
{
	if (-1 == len) len = Len(text);
	char* data = ExtEventCreate(id, extEventTypeText, sizeof(ulong_t) + sizeof(char_t) * (len + 1), p);
	if (NULL == data)
		return memErrNotEnoughSpace;
	ExtEventTextContents* contents = (ExtEventTextContents*)(data + sizeof(ExtEventData));
	contents->length = len;
	memmove(contents->data, text, len * sizeof(char_t));
	contents->data[len] = _T('\0');
	ExtEventSend(data, wnd);
	return errNone;
}

status_t ExtEventSendNarrow(ulong_t id, const char* text, long len, const Point* p, void* wnd)
{
	if (-1 == len) len = Len(text);
	char* data = ExtEventCreate(id, extEventTypeNarrowText, sizeof(ulong_t) + len + 1, p);
	if (NULL == data)
		return memErrNotEnoughSpace;
	ExtEventNarrowContents* contents = (ExtEventNarrowContents*)(data + sizeof(ExtEventData));
	contents->length = len;
	memmove(contents->data, text, len);
	contents->data[len] = '\0';
	ExtEventSend(data, wnd);
	return errNone;
}

status_t ExtEventSendBlob(ulong_t id, const void* blob, ulong_t len, const Point* p, void* wnd);

status_t ExtEventSendObject(ulong_t id, ExtEventObject* object, const Point* p, void* wnd)
{
	char* data = ExtEventCreate(id, extEventTypeObject, sizeof(ExtEventObjectContents), p);
	if (NULL == data)
		return memErrNotEnoughSpace;
	
	ExtEventObjectContents* contents = (ExtEventObjectContents*)(data + sizeof(ExtEventData));
	contents->object = object;
	ExtEventSend(data, wnd);
	return errNone;
}

#ifndef NDEBUG
static const char* testText = "tere fere kuku testujemy";
void test_ExtEventSend()
{
	status_t res = ExtEventSendNarrow(0, testText);
	assert(errNone == res);
}

void test_ExtEventReceive(Event& event)
{
	if (0 == ExtEventGetID(event))
	{
		const char* text = ExtEventGetNarrowText(event);
		assert(StrEquals(text, testText));
	}
}

#endif
