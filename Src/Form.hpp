#ifndef __ARSLEXIS_FORM_HPP__
#define __ARSLEXIS_FORM_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Geometry.hpp>
#include <Utility.hpp>
#include <FiveWay.hpp>

class PopupMenu;
class HyperlinkHandlerBase;

struct EventProperties;

class Application;

class FormGadget;
    
class Form: private NonCopyable
{
    Application& application_;
    FiveWay fiveWay_;
    UInt16 id_;
    FormType* form_;
    
    bool deleteOnClose_;
    bool deleteAfterEvent_;
    bool controlsAttached_;
    bool formOpenReceived_;
    bool winEnterReceived_;
    
    char_t * title_;
    UInt16 entryFocusControlId_;
    UInt16 focusedControlIndex_;
    UInt16 returnToFormId_;
    RectangleType screenBoundsBeforeWinExit_;
    
    FormGadget* trackingGadget_;
    
    static Boolean routeEventToForm(EventType* event);
    
    //bool handleFocusTransfer(EventType& event);
    
    bool handleObjectFocusChange(const EventType& event);
    
    void handleFocusOnEntry();

    PopupMenu* popupMenu_; 
    HyperlinkHandlerBase* popupMenuHyperlinkHandler_;
    
    uint_t popupMenuEventId_;
    uint_t popupMenuId_;
    void showPopupMenu(const char_t* txt, long len, const Point& point);
    
protected:

    virtual void attachControls();

    /**
     * Handles @c frmCloseEvent.
     * This function calls <code>delete this</code>, so when overriding it should be called last, 
     * unless you know what you're doing.
     */
    virtual bool handleClose();

    /**
     * Handles @c frmOpenEvent.
     * Updates form and prevents calling FrmDeleteForm() in destructor.
     */
    virtual bool handleOpen();

    /**
     * Handles @c frmUpdateEvent.
     * Calls redraw().
     */
    virtual bool handleUpdate(UInt16 updateCode);

    /**
     * Redraws form. 
     */
    virtual void draw(UInt16 updateCode = frmRedrawUpdateCode);

    virtual bool handleWindowEnter(const struct _WinEnterEventType&);
    
    virtual bool handleWindowExit(const struct _WinExitEventType& data);
    
    virtual bool handleEvent(EventType& event);
    
    virtual bool handleMenuCommand(UInt16)
    {return false;}
    
    virtual void afterGadgetDraw() {}
        
    virtual bool handleExtendedEvent(EventType& event);

public:

    enum {popupMenuEventInvalid = uint_t(-1)};
    
    void setPopupMenuEventId(uint_t id) {popupMenuEventId_ = id;}
    
    void setPopupMenuHyperlinkHandler(HyperlinkHandlerBase* hh) {popupMenuHyperlinkHandler_ = hh;}
    
    void setPopupMenuId(uint_t id) {popupMenuId_ = id;}
    
    void setupPopupMenu(uint_t id, uint_t eventId, HyperlinkHandlerBase* hh);
    
    Form(Application& app, UInt16 id);
    
    virtual Err initialize();
    
    virtual ~Form();
    
    WinHandle windowHandle() const
    {return FrmGetWindowHandle(form_);}
    
    Application& application()
    {return application_;}
    
    const Application& application() const
    {return application_;}
    
    void activate()
    {FrmSetActiveForm(form_);}
    
    UInt16 id() const 
    {return id_;}
    
    UInt16 showModal();
    
    void setFocusControlId(UInt16 focusControlId) {entryFocusControlId_ = focusControlId;}
    
    /**
     * Queues @c frmUpdateEvent for this form.
     */
    void update(UInt16 updateCode=frmRedrawUpdateCode);

    void returnToForm(UInt16 formId);

    void closePopup()
    {returnToForm(0);}
    
    operator const FormType* () const 
    {return form_;}
    
    operator FormType* ()
    {return form_;}
    
    bool visible() const
    {return FrmVisible(form_);}
    
    void bounds(ArsRectangle& out) const
    {
        RectangleType rect;
        FrmGetFormBounds(form_, &rect);
        out=rect;
    }
    
    ArsRectangle bounds() const
    {
        ArsRectangle rect;
        bounds(rect);
        return rect;
    }
    
    void setBounds(const ArsRectangle& bounds);

    UInt16 focusedObject() const
    {return FrmGetFocus(form_);}
    
    void releaseFocus();
    
    void setTitle(const char_t* title);
    
    const char_t *title();

    bool fiveWayLeftPressed(const EventType *event) const
    {return fiveWay_.LeftPressed(event);}

    bool fiveWayRightPressed(const EventType *event) const
    {return fiveWay_.RightPressed(event);}

    bool fiveWayUpPressed(const EventType *event) const 
    {return fiveWay_.UpPressed(event);}

    bool fiveWayDownPressed(const EventType *event) const
    {return fiveWay_.DownPressed(event);}
    
    bool fiveWayCenterPressed(const EventType* event) const
    {return fiveWay_.CenterPressed(event);}
    
    UInt16 getGraffitiStateIndex() const;
    
    void popup();
    
    void run();
    
    friend class Application;
    friend class FormGadget;
    friend class FormObject;
    friend class PopupRenderer;
    
    // @return index of new gadget or frmInvalidObjectId on failure.
    UInt16 createGadget(UInt16 id, const ArsRectangle& rect);
    
    UInt16 createList(UInt16 id, const ArsRectangle& rect, FontID font, Int16 visibleItems, Int16 triggerId = 0);
    
    void removeObject(UInt16 index) {FrmRemoveObject(&form_, index);}
    
};

#endif
