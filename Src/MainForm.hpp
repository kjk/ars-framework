#ifndef __MAINFORM_HPP__
#define __MAINFORM_HPP__

#include "iPediaForm.hpp"
#include "Definition.hpp"
#include <deque>

class MainForm: public iPediaForm
{
    enum {historyLength_=20};
    Definition definition_;
    ArsLexis::String term_;
    typedef std::deque<ArsLexis::String> TermHistory_t;
//    TermHistory_t termHistory_;
//    TermHistory_t::iterator historyPosition_; 
    
    void handleScrollRepeat(const sclRepeat& data);
    void handlePenUp(const EventType& event);
    void handleControlSelect(const ctlSelect& data);
    bool handleKeyPress(const EventType& event);
    
    void drawSplashScreen(ArsLexis::Graphics& graphics, ArsLexis::Rectangle& bounds);
    void drawDefinition(ArsLexis::Graphics& graphics, ArsLexis::Rectangle& bounds);
    void startLookupConnection(const ArsLexis::String& term);
    
    void updateCurrentTermField();
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
    
    void setTerm(const ArsLexis::String& term);
    
    const ArsLexis::String& term() const
    {return term_;}
    
    enum DisplayMode
    {
        showSplashScreen,
        showDefinition
    };
    
    DisplayMode displayMode() const
    {return displayMode_;}

/*    
    enum RedrawCommand
    {
        redrawAll=frmRedrawUpdateCode,
        redrawInputBar
    };
*/

    void setDisplayMode(DisplayMode displayMode)
    {displayMode_=displayMode;}
    
    void lookupTerm(const ArsLexis::String& term);
    
//    bool historyHasPrevious() const
//    {return termHistory_.begin()!=historyPosition_;}
    
//    bool historyHasNext() const
//    {return termHistory_.end()!=historyPosition_ && termHistory_.end()!=historyPosition_+1;}

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