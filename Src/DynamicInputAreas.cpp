#include "DynamicInputAreas.hpp"
#include "Form.hpp"
#include <SonyCLIE.h>

namespace ArsLexis 
{

    Boolean DIA_Support::tryInitSonySilkLib()
    {
        Err error=sonySilkLib_.initialize(sonySysLibNameSilk, sonySysFileCSilkLib);
        if (!error) {
            UInt32 vskVersion=0;
            error = FtrGet(sonySysFtrCreator, sonySysFtrNumVskVersion, &vskVersion);
            if (error) {
                error=SilkLibOpen(sonySilkLib_);
                if (!error) 
                    SilkLibEnableResize(sonySilkLib_);
            }
            else
            {
                if (vskVersionNum2==vskVersion) {
                    error=VskOpen(sonySilkLib_);
                    if (!error) 
                        VskSetState(sonySilkLib_, vskStateEnable, vskResizeVertically);			
                }
                else {
                    error=VskOpen(sonySilkLib_);
                    if (!error) 
                        VskSetState(sonySilkLib_, vskStateEnable, vskResizeHorizontally);			
                }
                if (!error)
                    sonyLibIsVsk_=true;
            }
            if (!error)
                hasSonySilkLib_=true;
        }
        return hasSonySilkLib();
    }

    DIA_Support::DIA_Support():
        hasPenInputMgr_(false),
        hasSonySilkLib_(false),
        sonyLibIsVsk_(false)
    {
        if (!tryInitSonySilkLib())
        {
            UInt32 value=0;
            Err error=FtrGet(pinCreator, pinFtrAPIVersion, &value);
            if (!error && value)
                hasPenInputMgr_=true;
        }
    }

    void DIA_Support::sonySilkLibDispose()
    {
        assert(hasSonySilkLib());
        if (sonyLibIsVsk_)
        {
            Err error=VskClose(sonySilkLib_);
            if (vskErrStillOpen==error)
                error=errNone;
            assert(!error);
        }
        else
        {
            SilkLibDisableResize(sonySilkLib_);
            Err error=SilkLibClose(sonySilkLib_);
            if (silkLibErrStillOpen==error)
                error=errNone;
            assert(!error);
        }
    }
    
    DIA_Support::~DIA_Support()
    {
        if (hasSonySilkLib())
            sonySilkLibDispose();
    }

    void DIA_Support::handleNotify() const
    {
        EventType event;
        MemSet(&event, sizeof(event), 0);
        event.eType=(eventsEnum)winDisplayChangedEvent;
        RectangleType& bounds=event.data.winDisplayChanged.newBounds;
        Coord x, y;
        WinGetDisplayExtent(&x, &y);
        bounds.extent.x=x;
        bounds.extent.y=y;
        EvtAddUniqueEventToQueue(&event, 0, true);
    }
    
    Err DIA_Support::configureForm(Form& form, Coord minH, Coord prefH, Coord maxH, Coord minW, Coord prefW, Coord maxW) const
    {
        FormType* formPtr=form;
        assert(formPtr);
        Err error=errNone;
        if (hasSonySilkLib())
            handleNotify(); 
        else if (hasPenInputManager())
        {
            error=FrmSetDIAPolicyAttr(formPtr, frmDIAPolicyCustom);
            if (!error)
            {
                error=PINSetInputTriggerState(pinInputTriggerEnabled);
                if (!error) 
                {
                    WinHandle wh=FrmGetWindowHandle(formPtr);
                    assert(wh);
                    error=WinSetConstraintsSize(wh, minH, prefH, maxH, minW, prefW, maxW);
                    if (!error)
                        error=PINSetInputAreaState(pinInputAreaUser);
                }
            }
        }
        if (pinErrNoSoftInputArea==error) 
            error=errNone;
        return error;
    }

}