#include "DefinitionParser.hpp"
#include "FormattedTextElement.hpp"
#include "ListNumberElement.hpp"
#include "BulletElement.hpp"
#include "ParagraphElement.hpp"
#include "HorizontalLineElement.hpp"
#include <cctype>
#include <memory>
#include <Utility.hpp>

using ArsLexis::String;
using ArsLexis::FontEffects;
using ArsLexis::startsWith;
using ArsLexis::startsWithIgnoreCase;

namespace {
    typedef std::auto_ptr<ParagraphElement> ParagraphPtr;
    typedef std::auto_ptr<GenericTextElement> TextPtr;
    typedef std::auto_ptr<DefinitionElement> ElementPtr;
}

DefinitionParser::DefinitionParser():
    openEmphasize_(false),
    openStrong_(false),
    openVeryStrong_(false),
    openTypewriter_(0),
    openSmall_(0),
    openStrikeout_(0),
    openUnderline_(0),
    openNowiki_(0),
    openSuperscript_(0),
    openSubscript_(0),
    text_(0),
    parsePosition_(0),
    lineEnd_(0),
    lastElementStart_(0),
    lastElementEnd_(0),
    unnamedLinksCount_(0),
    lineType_(emptyLine),
    previousLineType_(emptyLine),
    textPosition_(0)
{
}    

void DefinitionParser::clear()
{
    textLine_.clear();
    parentsStack_.clear();
    currentNumberedList_.clear();
    numListsStack_.clear();
    lastListNesting_.clear();
    definition_.clear();
}
    
DefinitionParser::~DefinitionParser()
{}

DefinitionElement* DefinitionParser::currentParent()
{
    DefinitionElement* result=0;
    if (!parentsStack_.empty())
        result=parentsStack_.back();
    return result;
}

void DefinitionParser::pushParent(DefinitionElement* parent)
{
    parentsStack_.push_back(parent);
}

void DefinitionParser::popParent()
{
    assert(!parentsStack_.empty());
    parentsStack_.pop_back();
}

void DefinitionParser::popAvailableParent()
{
    if (!parentsStack_.empty())
        parentsStack_.pop_back();
}

inline bool DefinitionParser::isPlainText() const
{
    return !(openEmphasize_ || openStrong_ || openVeryStrong_ || openTypewriter_ ||
        openSmall_ || openStrikeout_ || openUnderline_ || openSubscript_ || openSuperscript_);
}

void DefinitionParser::applyCurrentFormatting(FormattedTextElement* element)
{
    assert(element);
    FontEffects& fontEffects=element->fontEffects();
    if (openEmphasize_)
        fontEffects.setItalic(true);
    if (openStrong_)
        fontEffects.setWeight(FontEffects::weightBold);
    if (openVeryStrong_)
        fontEffects.setWeight(FontEffects::weightBlack);
    if (openSmall_)
        fontEffects.setSmall(true);
    if (openStrikeout_)
        fontEffects.setStrikeOut(true);
    if (openUnderline_)
        fontEffects.setUnderline(FontEffects::underlineSolid);
    if (openSubscript_)
        fontEffects.setSubscript(true);
    if (openSuperscript_)
        fontEffects.setSuperscript(true);
}

namespace {

    static const char entityReferenceStart='&';
    static const char entityReferenceEnd=';';

}    

void DefinitionParser::decodeHTMLCharacterEntityRefs(String& text) const
{
    uint_t length=text.length();
    uint_t index=0;
    bool inEntity=false;
    uint_t entityStart=0;
    while (index<length)
    {
        char chr=text[index];
        if (!inEntity && chr==entityReferenceStart)
        {
            inEntity=true;
            entityStart=index;
        }
        else if (inEntity)
        {
            if (chr==entityReferenceEnd)
            {
                String entity(text, entityStart+1, index-entityStart-1);
                if (!entity.empty() && entity[0]=='#')
                {
                    Int32 numVal=StrAToI(entity.c_str()+1);
                    if (numVal<0 || numVal>255)
                        chr=chrNull;
                    else
                        chr=(unsigned char)numVal;
                }
                else
                    chr=decoder_.decode(entity.c_str());
                if (chr)
                {
                    text.replace(entityStart, index-entityStart+1, &chr, 1);
                    length=text.length();
                    index=entityStart;
                }
                inEntity=false;
            }
            else if (!(chr=='#' || std::isalnum(chr)))
                inEntity=false;
        }
        ++index;
    }
}

namespace {

    static const char indentLineChar=':';
    static const char bulletChar='*';
    static const char numberedListChar='#';
    static const char headerChar='=';
    static const char strongChar='\'';
    static const char htmlTagStart='<';
    static const char htmlTagEnd='>';
    static const char htmlClosingTagChar='/';
    static const char horizontalLineChar='-';
    static const char definitionListChar=';';
    static const char linkOpenChar='[';
    static const char linkCloseChar=']';

}

#define horizontalLineString "----"
#define sectionString "=="
#define subSectionString "==="
#define subSubSectionString "===="
#define listCharacters "*#:"

#define emphasizeText "''"
#define strongText "'''"
#define veryStrongText "''''"

bool DefinitionParser::detectStrongTag(uint_t end)
{
    bool isStrongTag=false;
    if (startsWith(textLine_, emphasizeText, textPosition_))
    {
        createTextElement();
        if (startsWith(textLine_, strongText, textPosition_))
        {
            isStrongTag=true;
            if (startsWith(textLine_, veryStrongText, textPosition_))
            {
                textPosition_+=StrLen(veryStrongText);
                openVeryStrong_=!openVeryStrong_;
            }
            else
            {
                textPosition_+=StrLen(strongText);
                openStrong_=!openStrong_;
            }
        }
        else
        {
            isStrongTag=true;
            textPosition_+=StrLen(emphasizeText);
            openEmphasize_=!openEmphasize_;
        }
    }
    return isStrongTag;
}

#define nowikiText "nowiki"
#define teleTypeText "tt"
#define lineBreakText "br"
#define smallText "small"
#define strikeOutText "strike"
#define underlineText "u"
#define subscriptText "sub"
#define superscriptText "sup"

//! @todo Implement DefinitionParser::detectHTMLTag()
bool DefinitionParser::detectHTMLTag(uint_t end)
{
    bool result=false;
    uint_t tagStart=textPosition_+1;
    bool isClosing=false;
    if (tagStart<end && htmlClosingTagChar==textLine_[tagStart])
    {
        ++tagStart;
        isClosing=true;
    }
    if (tagStart<end)
    {
        String::size_type tagEndPos=textLine_.find(htmlTagEnd, tagStart);
        uint_t tagEnd=(tagEndPos==textLine_.npos)?end:tagEndPos;
        if (tagEnd<end)
        {
            if (startsWithIgnoreCase(textLine_, nowikiText, tagStart))
            {
                createTextElement();
                openNowiki_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(textLine_, teleTypeText, tagStart))
            {
                createTextElement();
                openTypewriter_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(textLine_, lineBreakText, tagStart))
            {
                createTextElement();
                std::auto_ptr<LineBreakElement> p(new LineBreakElement());
                appendElement(p.get());
                p.release();
                result=true;
            }
            else if (startsWithIgnoreCase(textLine_, smallText, tagStart))
            {
                createTextElement();
                openSmall_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(textLine_, strikeOutText, tagStart))
            {
                createTextElement();
                openStrikeout_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(textLine_, underlineText, tagStart))
            {
                createTextElement();
                openUnderline_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(textLine_, superscriptText, tagStart))
            {
                createTextElement();
                openSuperscript_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(textLine_, subscriptText, tagStart))
            {
                createTextElement();
                openSubscript_+=(isClosing?-1:1);
                result=true;
            }
        
            if (result)
                textPosition_=tagEnd+1;
        }
    }
    return result;
}

#define imagePrefix "image:"
#define mediaPrefix "media:"
#define linkCloseText "]"
#define linkPartSeparator "|"

bool DefinitionParser::detectHyperlink(uint_t end)
{

    bool isHyperlink=false;
    String::size_type linkEndPos=textLine_.find(linkCloseText, textPosition_+1);
    uint_t linkEnd=(textLine_.npos==linkEndPos?end:linkEndPos);
    if (linkEnd<end)
    {
        isHyperlink=true;
        createTextElement();
        if (linkOpenChar==textLine_[textPosition_+1] && linkEnd+1<end && linkCloseChar==textLine_[linkEnd+1]) // Is it link to other wikipedia article?
        {
            textPosition_+=2;
            bool isOtherLanguage=(textPosition_<linkEnd-2 && ':'==textLine_[textPosition_+2]);
            uint_t pastLinkEnd=linkEnd+2;
            // We don't want other language links as we provide only English contents, and we can't render names in some languages.
            if (!isOtherLanguage) 
            {
                String::size_type separatorPos=textLine_.find(linkPartSeparator, textPosition_);
                GenericTextElement* textElement;
                if (textLine_.npos==separatorPos || separatorPos>=linkEnd)
                {
                    lastElementStart_=textPosition_;
                    lastElementEnd_=linkEnd;
                    textElement=createTextElement();
                    if (textElement)
                        textElement->setHyperlink(textElement->text(), hyperlinkTerm);
                }
                else
                {
                    lastElementStart_=separatorPos+1;
                    lastElementEnd_=linkEnd;
                    textElement=createTextElement();
                    if (textElement)
                        textElement->setHyperlink(String(textLine_, textPosition_, separatorPos-textPosition_), hyperlinkTerm);
                }
                uint_t afterLinkTextEnd=pastLinkEnd;
                while (afterLinkTextEnd<end && (std::isalpha(textLine_[afterLinkTextEnd]) || '-'==textLine_[afterLinkTextEnd]))
                    ++afterLinkTextEnd;
                if (afterLinkTextEnd>pastLinkEnd && textElement)
                {
                    String textCopy=textElement->text();
                    textCopy.append(textLine_, pastLinkEnd, afterLinkTextEnd-pastLinkEnd);
                    textElement->swapText(textCopy);
                    pastLinkEnd=afterLinkTextEnd;
                }
            }                
            textPosition_=pastLinkEnd;
        }
        else // In case of external link...
        {
            ++textPosition_;
            String::size_type separatorPos=textLine_.find(" ", textPosition_);
            if (textLine_.npos==separatorPos || separatorPos>=linkEnd)
            {
                ++unnamedLinksCount_;
                char buffer[8];
                StrPrintF(buffer, "[%hu]", unnamedLinksCount_);
                lastElementStart_=textPosition_;
                lastElementEnd_=linkEnd;
                GenericTextElement* textElement=createTextElement();
                if (textElement)
                {
                    textElement->setHyperlink(textElement->text(), hyperlinkExternal);
                    String buffStr(buffer);
                    textElement->swapText(buffStr);
                }
            }
            else
            {
                lastElementStart_=separatorPos+1;
                lastElementEnd_=linkEnd;
                GenericTextElement* textElement=createTextElement();
                if (textElement)
                    textElement->setHyperlink(String(textLine_, textPosition_, separatorPos-textPosition_), hyperlinkExternal);
            }
            textPosition_=linkEnd+1;
        }
    }
    return isHyperlink;
}

namespace {

    inline static bool isNewline(char chr)
    {
        return chr=='\n';
    }

}

void DefinitionParser::parseText(uint_t end, ElementStyle style)
{
    openEmphasize_=false;
    openStrong_=false;
    openVeryStrong_=false;
    openTypewriter_=0;
    openSmall_=0;
    openStrikeout_=0;
    openUnderline_=0;
    openSuperscript_=0;
    openSubscript_=0;
    currentStyle_=style;

    if (end<parsePosition_)
        return;

    uint_t length=end-parsePosition_;
    while (length && std::isspace((*text_)[parsePosition_+length-1]))
        --length;
    textLine_.assign(*text_, parsePosition_, length);
    parsePosition_=end;
    {   
        String::iterator it=textLine_.begin();
        String::iterator end=textLine_.end();
        while (true)
        {
            it=std::find_if(it, end, isNewline);
            if (it==end)
                break;
            else
                *it=' ';
        }
    }        
    
    lastElementStart_=textPosition_=0;
    while (textPosition_<length)
    {
        char chr=textLine_[textPosition_];
        bool specialChar=false;
        lastElementEnd_=textPosition_;
        if ((htmlTagStart==chr && detectHTMLTag(length)) ||
            (0==openNowiki_ && 
                ((strongChar==chr && detectStrongTag(length)) ||
                (linkOpenChar==chr && detectHyperlink(length)))))
        {
            lastElementStart_=textPosition_;
            specialChar=true;
        }
                
        if (!specialChar)
            ++textPosition_;
    }
    lastElementEnd_=textPosition_;
    createTextElement();
}

GenericTextElement* DefinitionParser::createTextElement(const String& text, String::size_type start, String::size_type length)
{
    String copy(text, start, length);
    decodeHTMLCharacterEntityRefs(copy);
    TextPtr textElement;
    if (isPlainText())
        textElement=TextPtr(new GenericTextElement(copy));
    else
    {   
        std::auto_ptr<FormattedTextElement> element(new FormattedTextElement(copy));
        applyCurrentFormatting(element.get());
        textElement=element;
    } 
    appendElement(textElement.get());
    textElement->setStyle(currentStyle_);
    return textElement.release();    
}

GenericTextElement* DefinitionParser::createTextElement()
{
    GenericTextElement* textElement=0;
    if (lastElementStart_<lastElementEnd_)
        textElement=createTextElement(textLine_, lastElementStart_, lastElementEnd_-lastElementStart_);
    return textElement;
}

void DefinitionParser::startNewNumberedList(ListNumberElement* firstElement)
{
    if (currentNumberedList_.empty())
    {
        assert(numListsStack_.empty());
        currentNumberedList_.push_back(firstElement);
    }
    else 
    {
        numListsStack_.push_back(NumberedList_t(1, firstElement));
        currentNumberedList_.swap(numListsStack_.back());
    }
}

void DefinitionParser::finishCurrentNumberedList()
{
    assert(!currentNumberedList_.empty());
    uint_t totalCount=currentNumberedList_.size();
    for (uint_t i=0; i<totalCount; ++i)
        currentNumberedList_[i]->setTotalCount(totalCount);
    if (!numListsStack_.empty())
    {
        currentNumberedList_.swap(numListsStack_.back());
        numListsStack_.pop_back();
    }
    else
        currentNumberedList_.clear();
}

void DefinitionParser::manageListNesting(const String& newNesting)
{
    uint_t lastNestingDepth=lastListNesting_.length();
    uint_t newNestingDepth=newNesting.length();
    if (lastNestingDepth || newNestingDepth)
    {
        uint_t firstDiff=0;  // This will be index of first character that makes previous and current nesting descr. differ.
        while (firstDiff<std::min(lastNestingDepth, newNestingDepth) && 
            lastListNesting_[firstDiff]==newNesting[firstDiff])
            firstDiff++;
            
        if (lastNestingDepth>0)
        {
            for (uint_t i=lastNestingDepth; i>firstDiff; --i)
            {
                char listType=lastListNesting_[i-1];
                if (numberedListChar==listType)
                    finishCurrentNumberedList();
                popParent();
            }
        }
        
        if (newNestingDepth>0)
        {
            bool continueList=false;
            if (firstDiff==newNestingDepth) // Means we have just finished a sublist and next element will be another point in current list, not a sublist
            {
                assert(firstDiff>0); 
                popParent();  
                --firstDiff;                
                continueList=true;    // Mark that next created element should be continuation of existing list, not start of new one
            }
            for (uint_t i=firstDiff; i<newNestingDepth; ++i)
            {
                char elementType=newNesting[i];
                ElementPtr element;
                if (numberedListChar==elementType)
                {
                    if (continueList)
                    {
                        assert(!currentNumberedList_.empty());
                        std::auto_ptr<ListNumberElement> listElement(new ListNumberElement(currentNumberedList_.back()->number()+1));
                        currentNumberedList_.push_back(listElement.get());
                        element=listElement;
                    }
                    else
                    {
                        std::auto_ptr<ListNumberElement> listElement(new ListNumberElement(1));
                        startNewNumberedList(listElement.get());
                        element=listElement;
                    }                    
                }
                else if (bulletChar==elementType)
                    element=ElementPtr(new BulletElement());
                else 
                    element=ElementPtr(new IndentedParagraphElement());
                appendElement(element.get());
                pushParent(element.release());
                continueList=false;
            }
        }
    }
    lastListNesting_=newNesting;
}

void DefinitionParser::appendElement(DefinitionElement* element)
{
    element->setParent(currentParent());
    definition_.appendElement(element);
}

DefinitionParser::LineType DefinitionParser::detectLineType(uint_t start, uint_t end) const
{
    LineType lineType=textLine;
    if (0==openNowiki_)
    {
        if (end==start || (end-start==1 && (*text_)[start]=='\r'))
            lineType=emptyLine;
        else {
            switch ((*text_)[start])
            {
                case indentLineChar:
                case bulletChar:
                case numberedListChar:
                    lineType=listElementLine;
                    break;
                
                case definitionListChar:
                    lineType=definitionListLine;
                    break;
                
                case headerChar:
                    if (startsWith(*text_, sectionString, start))
                        lineType=headerLine;
                    break;
                    
                case horizontalLineChar:
                    if (startsWith(*text_, horizontalLineString, start))
                        lineType=horizontalBreakLine;
                    break;
            }       
        }
    }
    return lineType;
}

bool DefinitionParser::detectNextLine(uint_t textEnd, bool finish)
{
    String::size_type end=text_->find('\n', parsePosition_);
    bool goOn=(text_->npos!=end && end<textEnd);
    if (finish || goOn)
    {
        LineType previousLineType=lineType_;
        uint_t lineEnd=((text_->npos==end || end>=textEnd)?textEnd:end);
        LineType lineType=detectLineType(parsePosition_, lineEnd);
        if (textLine==lineType)
        {
            goOn=false;
            while (lineEnd+1<textEnd || finish)
            {
                end=text_->find('\n', lineEnd+1);
                if (!finish && (text_->npos==end || end>=textEnd))
                    break;
                if (finish && lineEnd==textEnd)
                    goto LineFinished;
                uint_t nextLineEnd=((text_->npos==end || end>=textEnd)?textEnd:end);
                if (textLine==detectLineType(lineEnd+1, nextLineEnd))
                    lineEnd=nextLineEnd;
                else
                {
                    goOn=true;
                    goto LineFinished;
                }
            }
        }
        else
        {
LineFinished:        
            previousLineType_=previousLineType;
            lineEnd_=lineEnd;
            lineType_=lineType;
        }
    }
    return goOn;
}


void DefinitionParser::parseTextLine()
{
    if (!lineAllowsContinuation(previousLineType_) )
    {
        ParagraphPtr para(new ParagraphElement());
        appendElement(para.get());
        pushParent(para.release());
    }
    parseText(lineEnd_, styleDefault);                
}

Err DefinitionParser::handleIncrement(const String& text, ulong_t& length, bool finish)
{
    Err error=errNone;
    ErrTry {
        text_=&text;
        parsePosition_=0;
        lineEnd_=0;
        bool goOn=false;
        do 
        {
#ifndef NDEBUG    
            const char* text=text_->data()+parsePosition_;
#endif        
            goOn=detectNextLine(length, finish);
            if (goOn || finish)
            {
                if (lineAllowsContinuation(previousLineType_) && textLine!=lineType_)
                    popParent(); 
                    
                if (listElementLine==previousLineType_ && listElementLine!=lineType_)
                    manageListNesting(String());
                    
                switch (lineType_)
                {
                    case headerLine:
                        parseHeaderLine();
                        break;
                        
                    case textLine:
                        parseTextLine();
                        break;                    
                    
                    case horizontalBreakLine:
                        appendElement(new HorizontalLineElement());
                        break;
                        
                    case emptyLine:
                        appendElement(new LineBreakElement());
                        break;
                        
                    case listElementLine:
                        parseListElementLine();
                        break;
                        
                    case definitionListLine:
                        parseDefinitionListLine();
                        break;
                        
                    default:
                        assert(false);        
                }
                parsePosition_=lineEnd_+1;
            }
        } while (goOn);
        if (finish && emptyLine!=lineType_)
        {
            if (lineAllowsContinuation(previousLineType_))
                popParent(); 
                
            if (listElementLine==previousLineType_)
                manageListNesting(String());
            
            assert(numListsStack_.empty());
            assert(currentNumberedList_.empty());            
        }
        if (!finish)
            length=parsePosition_;
    }
    ErrCatch (ex) {
        clear();
        error=ex;
    } ErrEndCatch        
    return error;
}

void DefinitionParser::updateDefinition(Definition& definition)
{
    Definition::HyperlinkHandler* handler=definition.hyperlinkHandler();
    definition.swap(definition_); 
    definition.setHyperlinkHandler(handler);
}

//! @todo Add header indexing
void DefinitionParser::parseHeaderLine()
{
    while (parsePosition_<lineEnd_ && (headerChar==(*text_)[parsePosition_] || std::isspace((*text_)[parsePosition_])))
        ++parsePosition_;
    uint_t lineEnd=lineEnd_;
    while (lineEnd>parsePosition_ && (headerChar==(*text_)[lineEnd-1] || std::isspace((*text_)[lineEnd-1])))
        --lineEnd;
    ParagraphPtr para(new ParagraphElement());
    appendElement(para.get());
    pushParent(para.release());
    parseText(lineEnd, styleHeader);
    if (!lineAllowsContinuation(headerLine))
        popParent();
}

void DefinitionParser::parseListElementLine()
{
    String::size_type startLong=text_->find_first_not_of(listCharacters, parsePosition_);
    uint_t start=lineEnd_;
    if (text_->npos!=startLong && startLong<lineEnd_)
        start=startLong;
    String elementDesc(*text_, parsePosition_, start-parsePosition_);
    manageListNesting(elementDesc);
    parsePosition_=start;
    while (parsePosition_<lineEnd_ && std::isspace((*text_)[parsePosition_]))
        ++parsePosition_;
    parseText(lineEnd_, styleDefault);
}

void DefinitionParser::parseDefinitionListLine()
{
    while (parsePosition_<lineEnd_ && (definitionListChar==(*text_)[parsePosition_] || std::isspace((*text_)[parsePosition_])))
        ++parsePosition_;
    uint_t lineEnd=lineEnd_;
    while (lineEnd>parsePosition_ && std::isspace((*text_)[lineEnd-1]))
        --lineEnd;
    ParagraphPtr para(new ParagraphElement());
    appendElement(para.get());
    pushParent(para.release());
    parseText(lineEnd, styleDefault);
    if (!lineAllowsContinuation(definitionListLine))
        popParent();
}
