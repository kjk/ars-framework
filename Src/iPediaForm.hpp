#ifndef __IPEDIAFORM_HPP__
#define __IPEDIAFORM_HPP__

#include "Form.hpp"
#include "iPediaApplication.hpp"


class iPediaForm: public ArsLexis::Form
{
protected:

    bool handleEvent(EventType& event);
    
    virtual void resize(const ArsLexis::Rectangle&)
    {}
    
public:

    enum RedrawCode
    {
        redrawAll=frmRedrawUpdateCode,
        redrawProgressIndicator
    };
    
    iPediaForm(iPediaApplication& app, UInt16 formId):
        Form(app, formId)
    {}
    
    Err initialize();

};

#endif
