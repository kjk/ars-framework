#ifndef __MAINFORM_HPP__
#define __MAINFORM_HPP__

#include "iPediaForm.hpp"
#include "Definition.hpp"

class MainForm: public iPediaForm
{
    Definition definition_;
    
    void handleScrollRepeat(const sclRepeat& data);
    void handlePenUp(const EventType& event);
    void handleControlSelect(const ctlSelect& data);
    
protected:

    void resize(const RectangleType& screenBounds);
    
    void draw(UInt16 updateCode=frmRedrawUpdateCode);
    
    Err initialize();
    
    Boolean handleEvent(EventType& event);
    

public:
    
    MainForm(iPediaApplication& app):
        iPediaForm(app, mainForm)
    {}

    
};

#endif