#include "DynamicInputAreas.hpp"
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
                    flags_.set(flagSonyLibIsVsk);
            }
            if (!error)
                flags_.set(flagHasSonySilkLib);
        }
        return flags_.test(flagHasSonySilkLib);
    }

    DIA_Support::DIA_Support()
    {
        if (!tryInitSonySilkLib())
        {
            UInt32 value=0;
            Err error=FtrGet(pinCreator, pinFtrAPIVersion, &value);
            if (!error && value)
                flags_.set(flagHasPenInputMgr);
        }
    }

    void DIA_Support::sonySilkLibDispose()
    {
        assert(hasSonySilkLib());
        if (flags_.test(flagSonyLibIsVsk))
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
        WinGetBounds(WinGetDisplayWindow(), &bounds);
        EvtAddUniqueEventToQueue(&event, 0, true);
    }

}