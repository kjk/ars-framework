#ifndef __IPEDIAFORM_HPP__
#define __IPEDIAFORM_HPP__

#include "Form.hpp"
#include "iPediaApplication.hpp"


class iPediaForm: public ArsLexis::Form
{
protected:

    Boolean handleEvent(EventType& event);
    
    virtual void resize(const ArsLexis::Rectangle& screenBounds)
    {}

public:

    iPediaForm(iPediaApplication& app, UInt16 formId):
        Form(app, formId)
    {}

};


#endif
