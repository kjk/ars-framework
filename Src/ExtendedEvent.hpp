#ifndef ARSLEXIS_EXTENDED_EVENT_HPP__
#define ARSLEXIS_EXTENDED_EVENT_HPP__

#include <Application.hpp>

static const eventsEnum extEvent = eventsEnum(ArsLexis::Application::appExtendedEvent);

#define EVT_MAGIC_NUMBER 90110705415L
#define EXT_EVT_TEXT_TYPE  1

void *    createExtendedEventText(ulong_t eventId, const ArsLexis::char_t *txt);
void      sendExtendedEvent(void *eventData);
ulong_t   getExtendedEventId(void *eventData);
ulong_t   getExtendedEventId(EventType *event);
ulong_t   getExtendedEventMagicNumber(void *eventData);
ulong_t   getExtendedEventType(void *eventData);
ArsLexis::char_t *getTextEventDataCopy(void *eventData);
ArsLexis::char_t *getTextEventDataCopy(EventType *event);
void freeExtendedEvent(EventType *event);
void sendTextEvent(ulong_t eventId, const ArsLexis::char_t *txt);

#endif
