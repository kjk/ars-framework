#ifndef __ARSLEXIS_RICH_FORM_HPP__
#define __ARSLEXIS_RICH_FORM_HPP__

#include <Form.hpp>

namespace ArsLexis {

    class RichApplication;

    class RichForm: public Form {
    
        bool disableDiaTrigger_;
    
    public:    
    
        RichForm(RichApplication& app, uint_t formId, bool disableDiaTrigger=false);
        
        ~RichForm();
        
        Err initialize();
        
    protected:
    
        bool handleEvent(EventType& event);
        
        virtual void resize(const ArsLexis::Rectangle&);

        WindowFormatType windowFormat() const;
        
    };

}

#endif