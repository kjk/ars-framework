#ifndef __MAINFORM_HPP__
#define __MAINFORM_HPP__

#include "iPediaForm.hpp"
#include "Definition.hpp"

class MainForm: public iPediaForm
{
    Definition definition_;
    
protected:

    void resize(const RectangleType& screenBounds);
    
    void draw(UInt16 updateCode=frmRedrawUpdateCode);
    
    Err initialize();

public:
    
    MainForm(iPediaApplication& app):
        iPediaForm(app, mainForm)
    {}

    
};

#endif