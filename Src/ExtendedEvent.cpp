#include <ExtendedEvent.hpp>
#include <SysUtils.hpp>

using namespace ArsLexis;

void ExtendedEventData::dispose() 
{
    if (NULL != properties) 
    {
        properties->dispose();
        properties = NULL;
    }
}

void EventProperties::dispose()
{
    delete this;
}

EventProperties::~EventProperties()
{
}

ExtendedEventData::ExtendedEventData():
    eventType(0),
    properties(NULL)
{}

TextEventProperties* TextEventProperties::create(const String& text)
{
    TextEventProperties* p = new TextEventProperties;
    p->magicNumber = magicNumberConstant;
    p->text.assign(text);
    return p;
}

TextEventProperties::~TextEventProperties()
{
}

void ArsLexis::sendExtendedEvent(uint_t eventType, EventProperties* properties)
{
    ExtendedEventData data;
    data.eventType = eventType;
    data.properties = properties;
    sendEvent(extEvent, data);
}


void ArsLexis::sendTextEvent(uint_t eventType, const String& text)
{
    TextEventProperties* p = TextEventProperties::create(text);
    sendExtendedEvent(eventType, p);
}

const String& ArsLexis::extractEventText(const EventType& event)
{
    assert(extEvent == event.eType);
    const ExtendedEventData& data = reinterpret_cast<const ExtendedEventData&>(event.data);
    assert(NULL != data.properties);
    assert(TextEventProperties::magicNumberConstant == data.properties->magicNumber);
    TextEventProperties* p = static_cast<TextEventProperties*>(data.properties);
    return p->text;    
}

bool ArsLexis::isExtendedEvent(const EventType& event, uint_t eventType)
{
    if (extEvent != event.eType)
        return false;
    const ExtendedEventData& data = reinterpret_cast<const ExtendedEventData&>(event.data);
    return data.eventType == eventType;
}
