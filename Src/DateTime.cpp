#include <DateTime.hpp>
#include <DateTime.h>

namespace {

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
    
}

bool ArsLexis::validateDate(const DateTimeType& date) 
{
    return 
        date.second>=0 && date.second<60 &&
        date.minute>=0 && date.minute<60 &&
        date.hour>=0 && date.hour<24 &&
        date.day>=1 && date.day<=31 &&
        date.month>=january && date.month<=december &&
        date.weekDay>=sunday && date.weekDay<=saturday;
}

Err ArsLexis::formatRfc822Date(const DateTimeType& date, String& out) 
{
    if (!validateDate(date))
        return sysErrParamErr;
    static const int rfc822Length=29;
    char_t buffer[rfc822Length+1];
    const char_t* weekDay=weekDayAbbrevs[date.weekDay];
    const char_t* month=monthAbbrevs[date.month-1];
    int len=StrPrintF(buffer, "%s, %02hd %s %04hd %02hd:%02hd:%02hd GMT", weekDay, date.day, month, date.year, date.hour, date.minute, date.second);
    if (len<0)
        return memErrNotEnoughSpace;
    assert(rfc822Length==len);
    out.assign(buffer, rfc822Length);
    return errNone;
}
