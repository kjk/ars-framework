#include <FormGadget.hpp>
#include <Graphics.hpp>

using namespace ArsLexis;

FormGadget::FormGadget(Form& form, UInt16 id):
    FormObjectWrapper(form, id),
    visible_(false),
    usable_(false)
#ifdef __MWERKS__
    , gadgetHandlerThunk_(gadgetHandler)
#endif
{}        

FormGadget::~FormGadget()
{}

void FormGadget::setupGadget(FormType* form, UInt16 index)
{
    assert(valid());
    FrmSetGadgetData(form, index, this);
#ifdef __MWERKS__
    FormGadgetHandlerType* handler=reinterpret_cast<FormGadgetHandlerType*>(gadgetHandlerThunk_.thunkData);
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

Boolean FormGadget::gadgetHandler(FormGadgetTypeInCallback* g, UInt16 cmd, void* param)
{
    FormGadget* gadget=const_cast<FormGadget*>(static_cast<const FormGadget*>(g->data));
    assert(0!=gadget);
    assert(gadget->id()==g->id);
    gadget->visible_=g->attr.visible;
    gadget->usable_=g->attr.usable;
    return gadget->handleGadgetCommand(cmd, param);
}
   
void FormGadget::cleanup()
{}

void FormGadget::drawProxy()
{
    Graphics graphics(form()->windowHandle());
    Rectangle rect;
    bounds(rect);
    Graphics::ClipRectangleSetter setClip(graphics, rect);
    draw(graphics);
}

void FormGadget::draw(Graphics& graphics)
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

        default:
            assert(false);
    }
    return handled;
}

bool FormGadget::handleGadgetCommand(UInt16 command, void* param)
{
    bool handled=false;
    switch (command) 
    {
        case formGadgetDeleteCmd:
            cleanup();
            break;
            
        case formGadgetDrawCmd:
            drawProxy();
            handled=true;
            break;

        case formGadgetHandleEventCmd:
            assert(0!=param);
            handled=handleEvent(*static_cast<EventType*>(param));
            break;
    }
    return handled;
}

