#include <ExtendedEvent.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

using ArsLexis::char_t;

static void *buildDataForEvent(ulong_t eventId, ulong_t magicNumber, ulong_t type, char *eventData, ulong_t dataLen)
{
    char *data = (char*)malloc(sizeof(eventId)+sizeof(magicNumber)+sizeof(type)+dataLen);
    if (NULL==data) 
        return NULL;
    MemMove(data, &magicNumber, sizeof(magicNumber));
    data += sizeof(magicNumber);
    MemMove(data, (char*)&magicNumber, sizeof(magicNumber));
    data += sizeof(magicNumber);
    MemMove(data, (char*)&type, sizeof(type));
    data += sizeof(type);
    MemMove(data, eventData, dataLen);
    return (void*)data;
}  

void *createExtendedEventText(ulong_t eventId, const char_t *txt)
{
    return buildDataForEvent(eventId, EVT_MAGIC_NUMBER, EXT_EVT_TEXT_TYPE, (char*)txt, sizeof(char_t)*(tstrlen(txt)+1));
};

void sendTextEvent(ulong_t eventId, const ArsLexis::char_t *txt)
{
    void *data = createExtendedEventText(eventId, txt);
    if (NULL!=data)
        sendExtendedEvent(data);
}

void sendExtendedEvent(void *eventData)
{
    ArsLexis::sendEvent(extEvent, eventData);
}

ulong_t   getExtendedEventId(EventType *event)
{
    void *data = reinterpret_cast<void*>(&(event->data));    
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

char_t *getTextEventDataCopy(void *eventData)
{
    assert(EVT_MAGIC_NUMBER == getExtendedEventMagicNumber(eventData));
    char_t *data = (char_t*)((char*)eventData+sizeof(ulong_t)+sizeof(ulong_t)+sizeof(ulong_t));
    return ArsLexis::StringCopy2(data);
}   

char_t *getTextEventDataCopy(EventType *event)
{
    void *data = reinterpret_cast<void*>(&(event->data));    
    return getTextEventDataCopy(data);
}

void freeExtendedEvent(EventType *event)
{
    assert(extEvent == event.eType);
    void *data = reinterpret_cast<void*>(&(event->data));    
    free(data);
}

