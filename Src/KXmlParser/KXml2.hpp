#ifndef __KXML2_KXML2_HPP__
#define __KXML2_KXML2_HPP__

#include "..\Debug.hpp"
#include "..\BaseTypes.hpp"
#include "..\ErrBase.h"

namespace KXml2 {

    typedef ArsLexis::String String;
    typedef ArsLexis::char_t char_t; 
    typedef ArsLexis::status_t error_t;

    //Error values
    enum ErrorCodes
    {
         //TODO: make it work!!!
         eNoError = errNone,
         eExpectedCharNotEqual=kxmlErrorClass,     //read(c) if (a = read()) != c
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
         eUnexpectedType,           //nextTag()
         ePreconditionStartTag,     //nextText()
         eEndTagExpected,           //nextText()
         eIllegalType               //isWhitespace() (when wanted type is not what we have)
    };
}

#endif

