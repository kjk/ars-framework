#ifndef __MAINFORM_HPP__
#define __MAINFORM_HPP__

#include "iPediaForm.hpp"
#include "Definition.hpp"
#include "LookupHistory.hpp"

class MainForm: public iPediaForm
{
    Definition definition_;
    LookupHistory history_;
    
    void handleScrollRepeat(const sclRepeat& data);
    void handlePenUp(const EventType& event);
    void handleControlSelect(const ctlSelect& data);
    bool handleKeyPress(const EventType& event);
    
    void drawSplashScreen(ArsLexis::Graphics& graphics, ArsLexis::Rectangle& bounds);
    void drawDefinition(ArsLexis::Graphics& graphics, ArsLexis::Rectangle& bounds);
    
    void startLookupConnection(const ArsLexis::String& term);
    void startHistoryLookup(bool forward);
    
    void updateScrollBar();
    
protected:

    void resize(const ArsLexis::Rectangle& screenBounds);
    
    void draw(UInt16 updateCode=frmRedrawUpdateCode);
    
    Err initialize();
    
    Boolean handleEvent(EventType& event);
    
    Boolean handleOpen();

public:
    
    MainForm(iPediaApplication& app);
    
    ~MainForm();

    Definition& definition()
    {return definition_;}
    
    LookupHistory& history()
    {return history_;}    
    
    enum DisplayMode
    {
        showSplashScreen,
        showDefinition
    };
    
    DisplayMode displayMode() const
    {return displayMode_;}

    void setDisplayMode(DisplayMode displayMode)
    {displayMode_=displayMode;}
    
    void lookupTerm(const ArsLexis::String& term);
    
    void synchronizeWithHistory();

    enum ScrollUnit
    {
        scrollLine,
        scrollPage
    };
    
    void scrollDefinition(int units, ScrollUnit unit);
    
private:
    
    DisplayMode displayMode_;
    
};

#endif