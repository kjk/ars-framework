#include "DefinitionParser.hpp"
#include "FormattedTextElement.hpp"
#include "ListNumberElement.hpp"
#include "BulletElement.hpp"
#include "ParagraphElement.hpp"
#include "HorizontalLineElement.hpp"
#include <cctype>
#include <memory>
#include "Utility.hpp"

using ArsLexis::String;
using ArsLexis::FontEffects;
using ArsLexis::startsWith;
using ArsLexis::startsWithIgnoreCase;

DefinitionParser::DefinitionParser(const String& text, uint_t initialOffset):
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
    text_(text),
    parsePosition_(initialOffset),
    lineEnd_(0),
    lastElementStart_(0),
    lastElementEnd_(0),
    unnamedLinksCount_(0),
    lineType_(emptyLine),
    previousLineType_(emptyLine)
{
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

static const char entityReferenceStart='&';
static const char entityReferenceEnd=';';

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
    if (startsWith(text_, emphasizeText, parsePosition_))
    {
        createTextElement();
        if (startsWith(text_, strongText, parsePosition_))
        {
            isStrongTag=true;
            if (startsWith(text_, veryStrongText, parsePosition_))
            {
                parsePosition_+=StrLen(veryStrongText);
                openVeryStrong_=!openVeryStrong_;
            }
            else
            {
                parsePosition_+=StrLen(strongText);
                openStrong_=!openStrong_;
            }
        }
        else
        {
            isStrongTag=true;
            parsePosition_+=StrLen(emphasizeText);
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
    uint_t tagStart=parsePosition_+1;
    bool isClosing=false;
    if (tagStart<end && htmlClosingTagChar==text_[tagStart])
    {
        ++tagStart;
        isClosing=true;
    }
    if (tagStart<end)
    {
        String::size_type tagEndPos=text_.find(htmlTagEnd, tagStart);
        uint_t tagEnd=(tagEndPos==text_.npos)?end:tagEndPos;
        if (tagEnd<end)
        {
            if (startsWithIgnoreCase(text_, nowikiText, tagStart))
            {
                createTextElement();
                openNowiki_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(text_, teleTypeText, tagStart))
            {
                createTextElement();
                openTypewriter_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(text_, lineBreakText, tagStart))
            {
                createTextElement();
                appendElement(new LineBreakElement());
                result=true;
            }
            else if (startsWithIgnoreCase(text_, smallText, tagStart))
            {
                createTextElement();
                openSmall_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(text_, strikeOutText, tagStart))
            {
                createTextElement();
                openStrikeout_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(text_, underlineText, tagStart))
            {
                createTextElement();
                openUnderline_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(text_, superscriptText, tagStart))
            {
                createTextElement();
                openSuperscript_+=(isClosing?-1:1);
                result=true;
            }
            else if (startsWithIgnoreCase(text_, subscriptText, tagStart))
            {
                createTextElement();
                openSubscript_+=(isClosing?-1:1);
                result=true;
            }
        
            if (result)
                parsePosition_=tagEnd+1;
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
    String::size_type linkEndPos=text_.find(linkCloseText, parsePosition_+1);
    uint_t linkEnd=(text_.npos==linkEndPos?end:linkEndPos);
    if (linkEnd<end)
    {
        isHyperlink=true;
        createTextElement();
        if (linkOpenChar==text_[parsePosition_+1] && linkEnd+1<end && linkCloseChar==text_[linkEnd+1]) // Is it link to other wikipedia article?
        {
            parsePosition_+=2;
            bool isOtherLanguage=(parsePosition_<linkEnd-2 && ':'==text_[parsePosition_+2]);
            uint_t pastLinkEnd=linkEnd+2;
            // We don't want other language links as we provide only English contents, and we can't render names in some languages.
            if (!isOtherLanguage) 
            {
                String::size_type separatorPos=text_.find(linkPartSeparator, parsePosition_);
                GenericTextElement* textElement;
                if (text_.npos==separatorPos || separatorPos>=linkEnd)
                {
                    lastElementStart_=parsePosition_;
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
                        textElement->setHyperlink(String(text_, parsePosition_, separatorPos-parsePosition_), hyperlinkTerm);
                }
                uint_t afterLinkTextEnd=pastLinkEnd;
                while (afterLinkTextEnd<end && (std::isalpha(text_[afterLinkTextEnd]) || '-'==text_[afterLinkTextEnd]))
                    ++afterLinkTextEnd;
                if (afterLinkTextEnd>pastLinkEnd && textElement)
                {
                    String textCopy=textElement->text();
                    textCopy.append(text_, pastLinkEnd, afterLinkTextEnd-pastLinkEnd);
                    textElement->swapText(textCopy);
                    pastLinkEnd=afterLinkTextEnd;
                }
            }                
            parsePosition_=pastLinkEnd;
        }
        else // In case of external link...
        {
            ++parsePosition_;
            String::size_type separatorPos=text_.find(" ", parsePosition_);
            if (text_.npos==separatorPos || separatorPos>=linkEnd)
            {
                ++unnamedLinksCount_;
                char buffer[8];
                StrPrintF(buffer, "[%hu]", unnamedLinksCount_);
                lastElementStart_=parsePosition_;
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
                    textElement->setHyperlink(String(text_, parsePosition_, separatorPos-parsePosition_), hyperlinkExternal);
            }
            parsePosition_=linkEnd+1;
        }
    }
    return isHyperlink;
}

void DefinitionParser::parseText(uint_t end, ElementStyle style)
{
    currentStyle_=style;
    lastElementStart_=parsePosition_;
    while (parsePosition_<end)
    {
        char chr=text_[parsePosition_];
        bool specialChar=false;
        lastElementEnd_=parsePosition_;
        if ((htmlTagStart==chr && detectHTMLTag(end)) ||
            (0==openNowiki_ && 
                ((strongChar==chr && detectStrongTag(end)) ||
                (linkOpenChar==chr && detectHyperlink(end)))))
        {
            lastElementStart_=parsePosition_;
            specialChar=true;
        }
                
        if (!specialChar)
            ++parsePosition_;
    }
    lastElementEnd_=parsePosition_;
    createTextElement();
}

GenericTextElement* DefinitionParser::createTextElement()
{
    GenericTextElement* textElement=0;
    if (lastElementStart_<lastElementEnd_)
    {
        String text(text_, lastElementStart_, lastElementEnd_-lastElementStart_);
        decodeHTMLCharacterEntityRefs(text);
        if (isPlainText())
            textElement=new GenericTextElement(text);
        else
        {
            FormattedTextElement* element(new FormattedTextElement(text));
            applyCurrentFormatting(element);
            textElement=element;
        } 
        appendElement(textElement);
        textElement->setStyle(currentStyle_);
    }
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
                char elementType=newNesting[firstDiff];
                DefinitionElement* element=0;
                if (numberedListChar==elementType)
                {
                    if (continueList)
                    {
                        assert(!currentNumberedList_.empty());
                        ListNumberElement* listElement=new ListNumberElement(currentNumberedList_.back()->number()+1);
                        currentNumberedList_.push_back(listElement);
                        element=listElement;
                    }
                    else
                    {
                        ListNumberElement* listElement=new ListNumberElement(1);
                        startNewNumberedList(listElement);
                        element=listElement;
                    }                    
                }
                else if (bulletChar==elementType)
                    element=new BulletElement();
                else 
                    element=new IndentedParagraphElement();
                appendElement(element);
                pushParent(element);
                continueList=false;
            }
        }
        lastListNesting_=newNesting;
    }
}

void DefinitionParser::appendElement(DefinitionElement* element)
{
    element->setParent(currentParent());
    definition_.appendElement(element);
}

bool DefinitionParser::detectNextLine(uint_t textEnd, bool finish)
{
    String::size_type end=text_.find('\n', parsePosition_);
    bool goOn=(text_.npos!=end && end<textEnd);
    if (finish || goOn)
    {
        previousLineType_=lineType_;
        lineEnd_=((text_.npos==end || end>=textEnd)?textEnd:end);
        lineType_=textLine;
        if (0==openNowiki_)
        {
            if (lineEnd_==parsePosition_)
                lineType_=emptyLine;
            else {
                switch (text_[parsePosition_])
                {
                    case indentLineChar:
                    case bulletChar:
                    case numberedListChar:
                        lineType_=listElementLine;
                        break;
                    
                    case definitionListChar:
                        lineType_=definitionListLine;
                        break;
                    
                    case headerChar:
                        if (startsWith(text_, sectionString, parsePosition_))
                            lineType_=headerLine;
                        break;
                        
                    case horizontalLineChar:
                        if (startsWith(text_, horizontalLineString, parsePosition_))
                            lineType_=horizontalBreakLine;
                        break;
                }       
            }
        }
    }
    return goOn;
}

void DefinitionParser::parseTextLine()
{
    if (!lineAllowsContinuation(previousLineType_) )
    {
        ParagraphElement* para=new ParagraphElement();
        appendElement(para);
        pushParent(para);
    }
    parseText(lineEnd_, styleDefault);                
}

void DefinitionParser::parseIncrement(uint_t end, bool finish)
{
    bool goOn=false;
    do 
    {
        const char* text=text_.data()+parsePosition_;
        goOn=detectNextLine(end, finish);
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
    if (finish)
    {
        if (lineAllowsContinuation(previousLineType_))
            popParent(); 
            
        if (listElementLine==previousLineType_)
            manageListNesting(String());
        
        assert(numListsStack_.empty());
        assert(currentNumberedList_.empty());            
    }
}

void DefinitionParser::updateDefinition(Definition& definition)
{
    Definition::HyperlinkHandler* handler=definition.hyperlinkHandler();
    std::swap(definition, definition_); 
    definition.setHyperlinkHandler(handler);
}

//! @todo Add header indexing
void DefinitionParser::parseHeaderLine()
{
    while (parsePosition_<lineEnd_ && headerChar==text_[parsePosition_] || std::isspace(text_[parsePosition_]))
        ++parsePosition_;
    uint_t lineEnd=lineEnd_;
    while (lineEnd>parsePosition_ && headerChar==text_[lineEnd-1] || std::isspace(text_[lineEnd-1]))
        --lineEnd;
    ParagraphElement* para=new ParagraphElement();
    appendElement(para);
    pushParent(para);
    parseText(lineEnd, styleHeader);
    if (!lineAllowsContinuation(headerLine))
        popParent();
}

void DefinitionParser::parseListElementLine()
{
    String::size_type startLong=text_.find_first_not_of(listCharacters, parsePosition_);
    uint_t start=lineEnd_;
    if (text_.npos!=startLong && startLong<lineEnd_)
        start=startLong;
    String elementDesc(text_, parsePosition_, start-parsePosition_);
    manageListNesting(elementDesc);
    parsePosition_=start;
    while (parsePosition_<lineEnd_ && std::isspace(text_[parsePosition_]))
        ++parsePosition_;
    parseText(lineEnd_, styleDefault);
}

//! @todo Implement DefinitionParser::parseDefinitionListLine()
void DefinitionParser::parseDefinitionListLine()
{
}
