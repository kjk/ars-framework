#ifndef __FIVE_WAY_H__
#define __FIVE_WAY_H__

#include <PalmOS.h>
#include <PalmNavigator.h>
#include <68K/Hs.h>

class FiveWay
{
private:
    bool hasFiveWay_;
    bool hasHsNav_;

public:
    FiveWay();
    //bool CenterPressed(EventType *event);
    bool HsLeftPressed(const EventType *event);
    bool HsRightPressed(const EventType *event);
    bool HsUpPressed(const EventType *event);
    bool HsDownPressed(const EventType *event);

    bool LeftPressed(const EventType *event);
    bool RightPressed(const EventType *event);
    bool UpPressed(const EventType *event);
    bool DownPressed(const EventType *event);
};

#endif
