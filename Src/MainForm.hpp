#ifndef __MAINFORM_HPP__
#define __MAINFORM_HPP__

#include "iPediaForm.hpp"
#include "Definition.hpp"

class MainForm: public iPediaForm
{
    Definition definition_;
    ArsLexis::String term_;
    
    void handleScrollRepeat(const sclRepeat& data);
    void handlePenUp(const EventType& event);
    void handleControlSelect(const ctlSelect& data);
    
    void drawSplashScreen(ArsLexis::Rectangle& bounds);
    void drawDefinition(ArsLexis::Rectangle& bounds);
    void startLookupConnection(const ArsLexis::String& term);
    
protected:

    void resize(const RectangleType& screenBounds);
    
    void draw(UInt16 updateCode=frmRedrawUpdateCode);
    
    Err initialize();
    
    Boolean handleEvent(EventType& event);
    

public:
    
    MainForm(iPediaApplication& app);
    
    ~MainForm();

    Definition& definition()
    {return definition_;}
    
    void setTerm(const ArsLexis::String& term)
    {term_=term;}
    
    const ArsLexis::String& term() const
    {return term_;}
    
    enum DisplayMode
    {
        showSplashScreen,
        showDefinition
    };
    
    DisplayMode displayMode() const
    {return displayMode_;}
    
    void setDisplayMode(DisplayMode displayMode)
    {displayMode_=displayMode;}
    
private:
    
    DisplayMode displayMode_;
    
};

#endif