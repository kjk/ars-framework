#ifndef ARSLEXIS_EXTENDED_EVENT_HPP__
#define ARSLEXIS_EXTENDED_EVENT_HPP__

#include <Application.hpp>

namespace ArsLexis {

    static const eventsEnum extEvent = eventsEnum(Application::appExtendedEvent);

    struct EventProperties {
    
        ulong_t magicNumber;
    
        virtual void dispose();
        
        virtual ~EventProperties();
    };
    
    typedef void (* ExtendedEventDeleter)(EventProperties*);
    
    struct ExtendedEventData {
    
        uint_t eventType;
        
        EventProperties* properties;
        
        ExtendedEventData();
        
        void dispose();
        
    };
    
    void sendExtendedEvent(uint_t eventType, EventProperties* properties);


    struct TextEventProperties: public EventProperties
    {
        String text;
        
        enum {magicNumberConstant = ulong_t(90110705415)};
        
        static TextEventProperties* create(const String& text);
        
        ~TextEventProperties();
        
    };
    
    void sendTextEvent(uint_t eventType, const String& text);
    
    const String& extractEventText(const EventType& event);
    
    bool isExtendedEvent(const EventType& event, uint_t eventType);
    
}

#endif