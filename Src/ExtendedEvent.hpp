#ifndef ARSLEXIS_EXTENDED_EVENT_HPP__
#define ARSLEXIS_EXTENDED_EVENT_HPP__

#include <Application.hpp>

namespace ArsLexis {

    static const eventsEnum extEvent = eventsEnum(Application::appExtendedEvent);

    struct EventProperties {
    
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

}

#endif