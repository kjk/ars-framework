#ifndef __REGISTRATION_FORM_HPP__
#define __REGISTRATION_FORM_HPP__

#include "iPediaForm.hpp"

class RegistrationForm: public iPediaForm
{
    void handleControlSelect(const ctlSelect& data);

protected:

    void resize(const ArsLexis::Rectangle& screenBounds);
    
    bool handleEvent(EventType& event);
    
    bool handleOpen();
    
public:

    RegistrationForm(iPediaApplication& app):
        iPediaForm(app, registrationForm)
    {}    

};

#endif