#ifndef __KXML2_KXML2_HPP__
#define __KXML2_KXML2_HPP__

#include "..\BaseTypes.hpp"

namespace KXml2 {

    typedef ArsLexis::String String;
    typedef ArsLexis::char_t char_t; 

    // Other common stuff goes here...
    // Error code returned
#if defined(_PALM_OS)
    //typedef Err error_t;
    typedef unsigned long error_t;
#elif defined(_WIN32)
    //typedef HRESULT error_t;
    typedef unsigned long error_t;
#else
#error "What the hell?"
#endif
    //Error values
    enum ErrorCodes
    {
        //TODO: change 0x3000 to text from ErrBase.hpp
         eNoError = 0x3000,
         eExpectedCharNotEqual,     //read(c) if (a = read()) != c
         eUnterminatedEntityRef,    //unterminated entity ref
         eUnresolvedCode,           //pushEntity
         eNameExpected,             //readName
         eIndexOutOfBoundsException,//IndexOutOfBoundsException()
         eIllegalEmptyNamespace,    //adjustNsp()
         eIllegalAttributeName,     //adjustNsp()
         eUndefinedPrefix,          //adjustNsp()
         eDuplicateAttribute,       //adjustNsp()
         eIllegalTagName,           //adjustNsp()
         eUnexpectedEof,            //UNEXPECTED_EOF
         eAttrNameExpected,         //parseStartTag()
         eInvalidDelimiter,         //parseStartTag()
         eNoInputSpecified,         //nextImpl()
         eNoReader,                 //when reader_ == NULL
         eElementStackEmpty,        //no elements on stack
         eExpectedDifferentName,    //parseEndTag()
         eTextNotAllowedOutsideRootElement, //text
         ePIMustNotStartWithXml,    //parseLegacy()
         eVersionExpected,          //parseLegacy()
         eIllegalStandaloneValue,   //parseLegacy()
         eIllegalXmldecl,           //parseLegacy()
         eIllegalStartTag,          //parseLegacy()
         eIllegalCommentDelimiter,  //parseLegacy()
         eUnsupportedFeature,       //setFeature()

         errr   



    };
}

#endif

