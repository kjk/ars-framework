#ifndef __ARSLEXIS_DATE_TIME_HPP__
#define __ARSLEXIS_DATE_TIME_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    bool validateDate(const DateTimeType& date);

    Err formatRfc822Date(const DateTimeType& date, String& out);
    
}

#endif