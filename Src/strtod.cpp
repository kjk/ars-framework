/***********************************************************************
 strtod.c - Defines a function to turn a numeric string into a double.

 Obtained from http://cold.40-below.com/palm/strtod.html

 This version was formatted and tweaked slightly by Warren Young
    <tangent@cyberport.com>

 Code is in the public domain, per this snippet from an email from
 David Bray <dbray@twcny.rr.com>:

    "You can consider fptest.c and the associated routines to be in
    the public domain."
    
 Code edited 2001-01-14 by Ben Combee <bcombee@metrowerks.com> to
 work with CodeWarrior for Palm OS 7 and 8.
***********************************************************************/

#include <Text.hpp>

// Routine to convert a string to a double -- 
// Allowed input is in fixed notation ddd.fff
// This does not use MathLib.
//
// Return true if conversion was successful, false otherwise

#define PalmIsDigit(c) ((c) >= '0' && (c) <= '9')

bool
strToDouble(const char* str, double *dbl)
{
    Int16 i, start, length, punctPos;
    double result, sign, fractPart;

    result = fractPart = 0.0;
    length = punctPos = StrLen(str);

    start = 0;
    sign = 1.0;
    if (str[0] == '-') {
        sign = -1.0;
        start = 1;
    }

    for (i = start; i < length; i++)    // parse the string from left to right converting the integer part
    {
        if (str[i] != '.') {
            if (PalmIsDigit(str[i]))
                result = result * 10.0 + (str[i] - '0');
            else {
                return false;
            }
        }
        else {
            punctPos = i;
            break;
        }
    }

    if (str[punctPos] == '.')   // parse the string from the end to the '.' converting the fractional part
    {
        for (i = length - 1; i > punctPos; i--) {
            if (PalmIsDigit(str[i]))
                fractPart = fractPart / 10.0 + (str[i] - '0');
            else {
                return false;
            }
        }
        result += fractPart / 10.0;
    }

    *dbl = result * sign;       // correcting the sign
	return true;
}

