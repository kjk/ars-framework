#include <ExtendedEvent.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

using namespace ArsLexis;

static void *buildDataForEvent(ulong_t eventId, ulong_t magicNumber, ulong_t type, char *eventData, ulong_t dataLen)
{
    ulong_t l =  sizeof(eventId)+sizeof(magicNumber)+sizeof(type)+dataLen;
    char* data = (char*)malloc(l);
    if (NULL == data) 
        return NULL;
    void* p = data;
    memmove(data, &eventId, sizeof(eventId));
    data += sizeof(eventId);
    memmove(data, &magicNumber, sizeof(magicNumber));
    data += sizeof(magicNumber);
    memmove(data, &type, sizeof(type));
    data += sizeof(type);
    memmove(data, eventData, dataLen);
    return p;
}  

void *createExtendedEventText(ulong_t eventId, const char_t* txt, ulong_t length)
{
    ulong_t l = sizeof(length) + (length + 1)*sizeof(char_t);
    char* data = (char*)malloc(l);
    if (NULL == data)
        return NULL;
    ulong_t* p = (ulong_t*)data;
    *p = length;
    data += sizeof(length);
    memmove(data, txt, length * sizeof(char_t));
    data += sizeof(char_t) * length;
    MemSet(data, sizeof(char_t), 0);
    void* res = buildDataForEvent(eventId, EVT_MAGIC_NUMBER, EXT_EVT_TEXT_TYPE, (char*)p, l);
    free(p);
    return res;
};

void sendTextEvent(ulong_t eventId, const ArsLexis::char_t* txt, const Point* point)
{
    void *data = createExtendedEventText(eventId, txt, tstrlen(txt));
    if (NULL!=data)
        sendExtendedEvent(data, point);
}

void sendTextNEvent(ulong_t eventId, const ArsLexis::char_t* txt, ulong_t length, const Point* point)
{
    void *data = createExtendedEventText(eventId, txt, length);
    if (NULL!=data)
        sendExtendedEvent(data, point);
}


void sendExtendedEvent(void* eventData, const Point* point)
{
    ArsLexis::sendEvent(extEvent, eventData, false, point);
}

ulong_t   getExtendedEventId(EventType *event)
{
    void *data = *reinterpret_cast<void**>(&event->data);  
    return getExtendedEventId(data);
}

ulong_t getExtendedEventId(void *eventData)
{
    assert(EVT_MAGIC_NUMBER == getExtendedEventMagicNumber(eventData));
    char *data = (char*)eventData;
    ulong_t *typePtr = (ulong_t*)data;
    return *typePtr;
}

ulong_t getExtendedEventMagicNumber(void *eventData)
{
    char *data = (char*)eventData+sizeof(ulong_t);
    ulong_t *magicPtr = (ulong_t*)data;
    return *magicPtr;
}

ulong_t getExtendedEventType(void *eventData)
{
    assert(EVT_MAGIC_NUMBER == getExtendedEventMagicNumber(eventData));
    char *data = (char*)eventData+sizeof(ulong_t)+sizeof(ulong_t);
    ulong_t *typePtr = (ulong_t*)data;
    return *typePtr;
}

const char_t* getTextEventData(EventType* event)
{
    void *eventData = *reinterpret_cast<void**>(&event->data);
    assert(EVT_MAGIC_NUMBER == getExtendedEventMagicNumber(eventData));
    const char_t *data = (const char_t*)((const char*)eventData+sizeof(ulong_t)+sizeof(ulong_t)+sizeof(ulong_t) + sizeof(ulong_t));
    return data;
}

ulong_t getTextEventDataLength(EventType* event)
{
    void *eventData = *reinterpret_cast<void**>(&event->data);
    assert(EVT_MAGIC_NUMBER == getExtendedEventMagicNumber(eventData));
    const ulong_t* data = (const ulong_t*)((const char*)eventData+sizeof(ulong_t)+sizeof(ulong_t)+sizeof(ulong_t));
    return *data;
}

char_t* getTextEventDataCopy(EventType *event, ulong_t& length)
{
    length = getTextEventDataLength(event);
    char_t* p = StringCopy2N(getTextEventData(event), length);
    return p;
}

void freeExtendedEvent(EventType *event)
{
    assert(extEvent == event->eType);
    void *data = *reinterpret_cast<void**>(&event->data);
    free(data);
}

