#include <ExtendedEvent.hpp>

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
