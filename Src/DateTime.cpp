#include <DateTime.hpp>
#include <DateTime.h>

enum {
    weekDayAbbrevLength=4,
    monthAbbrevLength=4
};

typedef ArsLexis::char_t WeekDayAbbrevStorage_t[weekDayAbbrevLength];

typedef ArsLexis::char_t MonthAbbrevStorage_t[monthAbbrevLength];

typedef WeekDayAbbrevStorage_t WeekDayAbbrevArray_t[daysInWeek];

typedef MonthAbbrevStorage_t MonthAbbrevArray_t[monthsInYear];

WeekDayAbbrevArray_t weekDayAbbrevs=
{
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

MonthAbbrevArray_t monthAbbrevs=
{
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};
    
bool validateDate(const DateTimeType& date) 
{

    if (! ((date.second>=0) && (date.second<60)))
        return false;

    if (! ((date.minute>=0) && (date.minute<60)) )
        return false;

    if (! ((date.hour>=0) && (date.hour<24)) )
        return false;

    if (! ((date.day>=1) && (date.day<=31)) )
        return false;
    
    if (! ((date.month>=january) && (date.month<=december)) )
        return false;

    if (! ((date.weekDay>=sunday) && (date.weekDay<=saturday)) )
        return false;

    return true;
}

#define rfc822Length 29

Err formatRfc822Date(const DateTimeType& date, String& out) 
{
    if (!validateDate(date))
        return sysErrParamErr;
    char_t buffer[rfc822Length+11]; // add 11 just to be sure
    const char_t* weekDay = weekDayAbbrevs[date.weekDay];
    const char_t* month = monthAbbrevs[date.month-1];
    int len = StrPrintF(buffer, "%s, %02hd %s %04hd %02hd:%02hd:%02hd GMT", weekDay, date.day, month, date.year, date.hour, date.minute, date.second);
    if (len<0)
        return memErrNotEnoughSpace;

    assert(rfc822Length==len);
    out.assign(buffer, rfc822Length);
    return errNone;
}
