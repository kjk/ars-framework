#ifndef __MAINFORM_HPP__
#define __MAINFORM_HPP__

#include "iPediaForm.hpp"
#include "Definition.hpp"
#include "LookupHistory.hpp"

class MainForm: public iPediaForm
{
    Definition definition_;
    LookupHistory history_;
    ArsLexis::String server_;
    
    void handleScrollRepeat(const sclRepeat& data);
    void handlePenUp(const EventType& event);
    void handleControlSelect(const ctlSelect& data);
    bool handleKeyPress(const EventType& event);
    
    void drawSplashScreen(ArsLexis::Graphics& graphics, ArsLexis::Rectangle& bounds);
    void drawDefinition(ArsLexis::Graphics& graphics, ArsLexis::Rectangle& bounds);
    
    void startLookupConnection(const ArsLexis::String& term);
    void startHistoryLookup(bool forward);
    
    void updateScrollBar();
    
    void copySelectionToClipboard();
    
protected:

    void resize(const ArsLexis::Rectangle& screenBounds);
    
    void draw(UInt16 updateCode=frmRedrawUpdateCode);
    
    Err initialize();
    
    bool handleEvent(EventType& event);
    
    bool handleOpen();
    
    bool handleMenuCommand(UInt16 itemId);

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
