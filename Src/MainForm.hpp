#ifndef __MAINFORM_HPP__
#define __MAINFORM_HPP__

#include "iPediaForm.hpp"

class MainForm: public iPediaForm
{
protected:

    void resize(const RectangleType& screenBounds);
    
    void draw();
    
public:
    
    MainForm(iPediaApplication& app):
        iPediaForm(app, mainForm)
    {}

    
};

#endif