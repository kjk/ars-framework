#include <FormGadget.hpp>
#include <Graphics.hpp>
#include <Application.hpp>

using namespace ArsLexis;

FormGadget::FormGadget(Form& form, UInt16 id):
    FormObjectWrapper(form, id),
    visible_(false),
    usable_(false),
    doubleBuffer_(false)
{}        

FormGadget::~FormGadget()
{}

void FormGadget::setupGadget(FormType* form, UInt16 index)
{
    assert(valid());
    FrmSetGadgetData(form, index, this);
#ifdef __MWERKS__
    FormGadgetHandlerType* handler=reinterpret_cast<FormGadgetHandlerType*>(Application::instance().gadgetHandlerThunk_.thunkData);
#else
    FormGadgetHandlerType* handler=gadgetHandler;
#endif
    FrmSetGadgetHandler(form, index, handler);
}

void FormGadget::attach(UInt16 id)
{
    FormObjectWrapper::attach(id);
    setupGadget(*form(), index());
}

void FormGadget::attachByIndex(UInt16 index)
{
    FormObjectWrapper::attachByIndex(index);
    setupGadget(*form(), index);
}

void FormGadget::drawProxy()
{
    if (!visible())
        return;
    if (form()->application().runningOnTreo600() && hasFocus())
        drawFocusRing();
    Graphics graphics(form()->windowHandle());
    Rectangle rect;
    bounds(rect);
    bool db=doubleBuffer_;
    if (db)
    {
        Rectangle formBounds;
        form()->bounds(formBounds);
        Err error;
        WinHandle wh = WinCreateOffscreenWindow(formBounds.width(), formBounds.height(), nativeFormat, &error);
        if (errNone == error)
        {
            {
                Graphics offscreen(wh);
                ActivateGraphics active(offscreen);
                handleDraw(graphics);
                offscreen.copyArea(rect, graphics, rect.topLeft);
            }
            WinDeleteWindow(wh, false);
        }
        else
            db=false;
    }
    if (!db)
    {
        Graphics::ClipRectangleSetter setClip(graphics, rect);
        handleDraw(graphics);
    }
}

void FormGadget::handleDraw(Graphics& graphics)
{}

bool FormGadget::handleEnter(const EventType& event)
{return false;}

bool FormGadget::handleMiscEvent(const EventType& event)
{return false;}

bool FormGadget::handleEvent(EventType& event)
{
    bool handled=false;
    switch (event.eType)
    {
        case frmGadgetEnterEvent:
            handled=handleEnter(event);
            break;
            
        case frmGadgetMiscEvent:
            handled=handleMiscEvent(event);
            break;
        
    }
    return handled;
}

bool FormGadget::handleGadgetCommand(UInt16 command, void* param)
{
    bool handled=false;
    switch (command) 
    {
        case formGadgetDrawCmd:
            drawProxy();
            handled=true;
            form()->afterGadgetDraw();
            break;

        case formGadgetHandleEventCmd:
            assert(0!=param);
            handled=handleEvent(*static_cast<EventType*>(param));
            break;
    }
    return handled;
}

void FormGadget::handleFocusChange(FocusChange change)
{
    if (form()->application().runningOnTreo600() && visible() && form()->visible()) 
    {
        if (focusTaking == change)
            drawFocusRing();
        else
            removeFocusRing();
    }
}

void FormGadget::notifyHide() {}

void FormGadget::notifyShow() {}

