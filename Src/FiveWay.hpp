#ifndef __FIVE_WAY_H__
#define __FIVE_WAY_H__

#include <PalmOS.h>

class FiveWay
{
private:
    bool hasFiveWay_;
    bool hasHsNav_;

public:

    FiveWay();

    //bool CenterPressed(EventType *event);
    bool HsLeftPressed(const EventType *event) const;
    bool HsRightPressed(const EventType *event) const;
    bool HsUpPressed(const EventType *event) const;
    bool HsDownPressed(const EventType *event) const;

    bool LeftPressed(const EventType *event) const;
    bool RightPressed(const EventType *event) const;
    bool UpPressed(const EventType *event) const;
    bool DownPressed(const EventType *event) const;
};

#endif
