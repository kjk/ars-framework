#include "FiveWay.hpp"
#include <PalmNavigator.h>
#include <68K/Hs.h>

FiveWay::FiveWay()
{
    UInt32      value;
    Err         err;

    hasFiveWay_ = false;
    hasHsNav_ = false;

    err = FtrGet( navFtrCreator, navFtrVersion, &value );
    if ( err == errNone )
        hasFiveWay_ = true;

    err = FtrGet( sysFileCSystem, sysFtrNumUIHardwareHas5Way, &value );
    if ( err == errNone )
    {
        hasFiveWay_ = true;
        err = FtrGet( hsFtrCreator, hsFtrIDNavigationSupported, &value );
        if ( err == errNone )
            hasHsNav_ = true;
    }
}

#define IsHsFiveWayNavEvent(eventP) \
( \
    hasHsNav_ && ((eventP)->eType == keyDownEvent) && \
    ( \
        ((((eventP)->data.keyDown.chr == vchrPageUp) || \
          ((eventP)->data.keyDown.chr == vchrPageDown)) && \
         (((eventP)->data.keyDown.modifiers & commandKeyMask) != 0)) \
        || \
        (TxtCharIsRockerKey((eventP)->data.keyDown.modifiers, \
                            (eventP)->data.keyDown.chr)) \
    ) \
)

#define HsNavDirectionPressed(eventP, nav) \
( \
  IsHsFiveWayNavEvent(eventP) && \
  ( vchrRocker ## nav == vchrRockerUp) ? \
   (((eventP)->data.keyDown.chr == vchrPageUp) || \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav)) : \
   (vchrRocker ## nav == vchrRockerDown) ? \
   (((eventP)->data.keyDown.chr == vchrPageDown) || \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav)) : \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav) \
)

inline bool FiveWay::HsLeftPressed(const EventType *event) const
{
    return HsNavDirectionPressed(event, Left);
}

inline bool FiveWay::HsRightPressed(const EventType *event) const
{
    return HsNavDirectionPressed(event, Right);
}

inline bool FiveWay::HsUpPressed(const EventType *event) const 
{
    return HsNavDirectionPressed(event, Up);
}

inline bool FiveWay::HsDownPressed(const EventType *event) const 
{
    return HsNavDirectionPressed(event, Down);
}

bool FiveWay::LeftPressed(const EventType *event) const 
{
    return NavDirectionPressed(event,Left) || HsLeftPressed(event); 
}

bool FiveWay::RightPressed(const EventType *event) const 
{
    return NavDirectionPressed(event,Right) || HsRightPressed(event);
}

bool FiveWay::UpPressed(const EventType *event) const 
{
    return NavDirectionPressed(event,Up) || HsUpPressed(event);
}

bool FiveWay::DownPressed(const EventType *event) const
{
    return NavDirectionPressed(event,Down) || HsDownPressed(event);
}

bool FiveWay::CenterPressed(const EventType *event) const
{
    return NavSelectPressed(event) || HsNavDirectionPressed(event, Center);
}

