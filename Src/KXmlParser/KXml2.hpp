#ifndef __KXML2_KXML2_HPP__
#define __KXML2_KXML2_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace KXml2 {

    using ArsLexis::String;
    using ArsLexis::char_t; 
    typedef ArsLexis::status_t error_t;

    //Error values
    enum ErrorCodes
    {
         //TODO: make it work!!!
         eNoError = errNone,
         eExpectedCharNotEqual=kxmlErrorClass,     //read(c) if (a = read()) != c, 0x8300
         eUnterminatedEntityRef,    //unterminated entity ref, 0x8301
         eUnresolvedCode,           //pushEntity, 0x8302
         eNameExpected,             //readName, 0x8303
         eIndexOutOfBoundsException,//IndexOutOfBoundsException(), 0x8304
         eIllegalEmptyNamespace,    //adjustNsp(), 0x8305
         eIllegalAttributeName,     //adjustNsp(), 0x8306
         eUndefinedPrefix,          //adjustNsp(), 0x8307
         eDuplicateAttribute,       //adjustNsp(), 0x8308
         eIllegalTagName,           //adjustNsp(), 0x8309
         eUnexpectedEof,            //UNEXPECTED_EOF, 0x830a
         eAttrNameExpected,         //parseStartTag(), 0x830b
         eInvalidDelimiter,         //parseStartTag(), 0x830c
         eNoInputSpecified,         //nextImpl(), 0x830d
         eNoReader,                 //when reader_ == NULL, 0x830e
         eElementStackEmpty,        //no elements on stack, 0x830f
         eExpectedDifferentName,    //parseEndTag(), 0x8310
         eTextNotAllowedOutsideRootElement, //text, 0x8311
         ePIMustNotStartWithXml,    //parseLegacy(), 0x8312
         eVersionExpected,          //parseLegacy(), 0x8313
         eIllegalStandaloneValue,   //parseLegacy(), 0x8314
         eIllegalXmldecl,           //parseLegacy(), 0x8315
         eIllegalStartTag,          //parseLegacy(), 0x8316
         eIllegalCommentDelimiter,  //parseLegacy(), 0x8317
         eUnsupportedFeature,       //setFeature(), 0x8318
         eUnexpectedType,           //nextTag(), 0x8319
         ePreconditionStartTag,     //nextText(), 0x831a
         eEndTagExpected,           //nextText(), 0x831b
         eIllegalType               //isWhitespace() (when wanted type is not what we have), 0x831c
    };
}

#endif

