#include "DynamicInputAreas.hpp"
#include <SonyCLIE.h>

namespace ArsLexis 
{

    Boolean DIA_Support::tryInitSonySilkLib() throw()
    {
        Err error=SysLibFind(sonySysLibNameSilk, &sonySilkLibRefNum_);
        if (sysErrLibNotFound==error) {
            error=SysLibLoad('libr', sonySysFileCSilkLib, &sonySilkLibRefNum_);
            if (!error) 
                flags_.set(flagLoadedSonySilkLib);
        }
        if (!error) {
            flags_.set(flagHasSonySilkLib);
            UInt32 vskVersion=0;
            error = FtrGet(sonySysFtrCreator, sonySysFtrNumVskVersion, &vskVersion);
            if (error) {
                error=SilkLibOpen(sonySilkLibRefNum_);
                if (!error) 
                    SilkLibEnableResize(sonySilkLibRefNum_);
            }
            else
            {
                if (vskVersionNum2==vskVersion) {
                    error=VskOpen(sonySilkLibRefNum_);
                    if (!error) 
                        VskSetState(sonySilkLibRefNum_, vskStateEnable, vskResizeVertically);			
                }
                else {
                    error=VskOpen(sonySilkLibRefNum_);
                    if (!error) 
                        VskSetState(sonySilkLibRefNum_, vskStateEnable, vskResizeHorizontally);			
                }
                if (!error)
                    flags_.set(flagSonyLibIsVsk);
            }
            if (!error)
                flags_.set(flagHasSonySilkLib);
            else if (flags_.test(flagLoadedSonySilkLib))
                SysLibRemove(sonySilkLibRefNum_);
        }
        return flags_.test(flagHasSonySilkLib);
    }

    DIA_Support::DIA_Support() throw():
        sonySilkLibRefNum_(0)
    {
        if (!tryInitSonySilkLib())
        {
            UInt32 value=0;
            Err error=FtrGet(pinCreator, pinFtrAPIVersion, &value);
            if (!error && value)
                flags_.set(flagHasPenInputMgr);
        }
    }

    void DIA_Support::sonySilkLibDispose() throw()
    {
        assert(hasSonySilkLib());
        if (flags_.test(flagSonyLibIsVsk))
        {
            Err error=VskClose(sonySilkLibRefNum_);
            if (vskErrStillOpen==error)
                error=errNone;
            assert(!error);
        }
        else
        {
            SilkLibDisableResize(sonySilkLibRefNum_);
            Err error=SilkLibClose(sonySilkLibRefNum_);
            if (silkLibErrStillOpen==error)
                error=errNone;
            assert(!error);
        }
        if (flags_.test(flagLoadedSonySilkLib))
        {
            Err error=SysLibRemove(sonySilkLibRefNum_);
            assert(!error);
        }
    }
    
    DIA_Support::~DIA_Support() throw()
    {
        if (hasSonySilkLib())
            sonySilkLibDispose();
    }

    void DIA_Support::handleNotify() const throw()
    {
        EventType event;
        MemSet(&event, sizeof(event), 0);
        event.eType=(eventsEnum)winDisplayChangedEvent;
        RectangleType& bounds=event.data.winDisplayChanged.newBounds;
        WinGetBounds(WinGetDisplayWindow(), &bounds);
        EvtAddUniqueEventToQueue(&event, 0, true);
    }

}