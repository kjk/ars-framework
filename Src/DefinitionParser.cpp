#include "DefinitionParser.hpp"
#include "FormattedTextElement.hpp"
#include "ListNumberElement.hpp"
#include "BulletElement.hpp"
#include "ParagraphElement.hpp"
#include "HorizontalLineElement.hpp"
#include <cctype>
#include "Utility.hpp"

using ArsLexis::String;
using ArsLexis::isWhitespace;

DefinitionParser::DefinitionParser():
    openEmphasize_(false),
    openStrong_(false),
    openVeryStrong_(false),
    openTypewriter_(0),
    openSmall_(0),
    openStrikeout_(0),
    openUnderline_(0),
    openNowiki_(0),
    parsePosition_(0),
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


Boolean DefinitionParser::isPlainText() const
{
    return !(openEmphasize_ || openStrong_ || openVeryStrong_ || openTypewriter_ ||
        openSmall_ || openStrikeout_ || openUnderline_);
}

void DefinitionParser::applyCurrentFormatting(FormattedTextElement* element)
{
    assert(element);
    FormattingProperties& formatting=element->formatting();
    if (openEmphasize_)
        formatting.setStrength(FormattingProperties::strengthEmphasize);
    if (openStrong_)
        formatting.setStrength(FormattingProperties::strengthStrong);
    if (openVeryStrong_)
        formatting.setStrength(FormattingProperties::strengthVeryStrong);
    if (openTypewriter_)
        formatting.setTypewriterFont(true);
    if (openSmall_)
        formatting.setSmall(true);
    if (openStrikeout_)
        formatting.setStrikeOut(true);
    if (openUnderline_)
        formatting.setUnderline(true);
}

void DefinitionParser::reset()
{
    openEmphasize_=false;
    openStrong_=false;
    openVeryStrong_=false;
    openTypewriter_=0;
    openSmall_=0;
    openStrikeout_=0;
    openUnderline_=0;
    openNowiki_=0;
    parsePosition_=0;
    lineEnd_=0;
    lineType_=emptyLine;
    previousLineType_=emptyLine;
    lastElementStart_=0;
    lastElementEnd_=0;
    unnamedLinksCount_=0;
    text_.clear();
}
    
    
void DefinitionParser::parse(const String& text, Definition& definition)
{
    text_=text;
    parse();
    definition.swap(definition_);
    definition_.clear();
    reset();
}

static const char entityReferenceStart='&';
static const char entityReferenceEnd=';';

void DefinitionParser::decodeHTMLCharacterEntityRefs(String& text) const
{
    UInt16 length=text.length();
    UInt16 index=0;
    Boolean inEntity=false;
    UInt16 entityStart=0;
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
                    chr=decoder_.decode(entity);
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
static const char horizontalLineChar='-';
static const char definitionListChar=';';
static const char linkOpenChar='[';
static const char linkCloseChar=']';

#define horizontalLineString "----"
#define sectionString "=="
#define subSectionString "==="
#define subSubSectionString "===="
#define listCharacters "*#"

#define emphasizeText "''"
#define strongText "'''"
#define veryStrongText "''''"

Boolean DefinitionParser::detectStrongTag(UInt16 end)
{
    Boolean isStrongTag=false;
    if (text_.find(emphasizeText, parsePosition_)==parsePosition_)
    {
        createTextElement();
        if (text_.find(strongText, parsePosition_)==parsePosition_)
        {
            isStrongTag=true;
            if (text_.find(veryStrongText, parsePosition_)==parsePosition_)
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
Boolean DefinitionParser::detectHTMLTag(UInt16 end)
{
    return false;
}

#define imagePrefix "image:"
#define mediaPrefix "media:"
#define linkCloseText "]"
#define linkPartSeparator "|"

Boolean DefinitionParser::detectHyperlink(UInt16 end)
{

    Boolean isHyperlink=false;
    String::size_type linkEndPos=text_.find(linkCloseText, parsePosition_+1);
    UInt16 linkEnd=(text_.npos==linkEndPos?end:linkEndPos);
    if (linkEnd<end)
    {
        isHyperlink=true;
        createTextElement();
        if (linkOpenChar==text_[parsePosition_+1] && linkEnd+1<end && linkCloseChar==text_[linkEnd+1]) // Is it link to other wikipedia article?
        {
            parsePosition_+=2;
            Boolean isOtherLanguage=(parsePosition_<linkEnd-2 && ':'==text_[parsePosition_+2]);
            // We don't want other language links as we provide only English contents, and we can't render names in some languages.
            if (!isOtherLanguage) 
            {
                String::size_type separatorPos=text_.find(linkPartSeparator, parsePosition_);
                if (text_.npos==separatorPos || separatorPos>=linkEnd)
                {
                    lastElementStart_=parsePosition_;
                    lastElementEnd_=linkEnd;
                    GenericTextElement* textElement=createTextElement();
                    if (textElement)
                        textElement->setHyperlink(textElement->text(), hyperlinkTerm);
                }
                else
                {
                    lastElementStart_=separatorPos+1;
                    lastElementEnd_=linkEnd;
                    GenericTextElement* textElement=createTextElement();
                    if (textElement)
                        textElement->setHyperlink(String(text_, parsePosition_, separatorPos-parsePosition_), hyperlinkTerm);
                }
            }                
            parsePosition_=linkEnd+2;
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

void DefinitionParser::parseText(UInt16 end, ElementStyle style)
{
    currentStyle_=style;
    lastElementStart_=parsePosition_;
    while (parsePosition_<end)
    {
        char chr=text_[parsePosition_];
        Boolean specialChar=false;
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
            FormattedTextElement* element=new FormattedTextElement(text);
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
    UInt16 totalCount=currentNumberedList_.size();
    for (UInt16 i=0; i<totalCount; ++i)
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
    UInt16 lastNestingDepth=lastListNesting_.length();
    UInt16 newNestingDepth=newNesting.length();
    if (lastNestingDepth || newNestingDepth)
    {
        UInt16 firstDiff=0;  // This will be index of first character that makes previous and current nesting descr. differ.
        while (firstDiff<std::min(lastNestingDepth, newNestingDepth) && 
            lastListNesting_[firstDiff]==newNesting[firstDiff])
            firstDiff++;
            
        if (lastNestingDepth>0)
        {
            for (UInt16 i=lastNestingDepth; i>firstDiff; --i)
            {
                char listType=lastListNesting_[i-1];
                if (numberedListChar==listType)
                    finishCurrentNumberedList();
                popParent();
            }
        }
        
        if (newNestingDepth>0)
        {
            Boolean continueList=false;
            if (firstDiff==newNestingDepth) // Means we have just finished a sublist and next element will be another point in current list, not a sublist
            {
                assert(firstDiff>0); 
                popParent();  
                --firstDiff;                
                continueList=true;    // Mark that next created element should be continuation of existing list, not start of new one
            }
            for (UInt16 i=firstDiff; i<newNestingDepth; ++i)
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
                else
                    element=new BulletElement();
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

void DefinitionParser::detectLineType()
{
    String::size_type end=text_.find('\n', parsePosition_);
    lineEnd_=(text_.npos==end?text_.length():end);
    
    lineType_=textLine;
    if (0==openNowiki_)
    {
        if (lineEnd_==parsePosition_)
            lineType_=emptyLine;
        else {
            switch (text_[parsePosition_])
            {
                case indentLineChar:
                    lineType_=indentedLine;
                    break;
                    
                case bulletChar:
                case numberedListChar:
                    lineType_=listElementLine;
                    break;
                
                case definitionListChar:
                    lineType_=definitionListLine;
                    break;
                
                case headerChar:
                    if (text_.find(sectionString, parsePosition_)==parsePosition_)
                        lineType_=headerLine;
                    break;
                    
                case horizontalLineChar:
                    if (text_.find(horizontalLineString, parsePosition_)==parsePosition_)
                        lineType_=horizontalBreakLine;
                    break;
            }       
        }
    }
}

void DefinitionParser::parseTextLine()
{
    if (!lineAllowsContinuation(previousLineType_))
    {
        ParagraphElement* para=new ParagraphElement();
        appendElement(para);
        pushParent(para);
    }
    parseText(lineEnd_, styleDefault);                
}

void DefinitionParser::parse()
{
    previousLineType_=lineType_;
    while (parsePosition_<text_.length())
    {
        detectLineType();

        if (lineAllowsContinuation(previousLineType_) && textLine!=lineType_ && listElementLine!=previousLineType_)
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
                
            case indentedLine:
                parseIndentedLine();
                break;
                
            case definitionListLine:
                parseDefinitionListLine();
                break;
                
            default:
                assert(false);        
        }
        parsePosition_=lineEnd_+1;
        previousLineType_=lineType_;
    }
}

//! @todo Add header indexing
void DefinitionParser::parseHeaderLine()
{
    while (parsePosition_<lineEnd_ && headerChar==text_[parsePosition_] || isWhitespace(text_[parsePosition_]))
        ++parsePosition_;
    UInt16 lineEnd=lineEnd_;
    while (lineEnd>parsePosition_ && headerChar==text_[lineEnd-1] || isWhitespace(text_[lineEnd-1]))
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
    UInt16 start=lineEnd_;
    if (text_.npos!=startLong && startLong<lineEnd_)
        start=startLong;
    String elementDesc(text_, parsePosition_, start-parsePosition_);
    manageListNesting(elementDesc);
    parsePosition_=start;
    while (parsePosition_<lineEnd_ && isWhitespace(text_[parsePosition_]))
        ++parsePosition_;
    parseText(lineEnd_, styleDefault);
    if (!lineAllowsContinuation(listElementLine))
        popParent();
}

void DefinitionParser::parseIndentedLine()
{
    assert(indentLineChar==text_[parsePosition_]);
    ++parsePosition_;
    while (parsePosition_<lineEnd_ && isWhitespace(text_[parsePosition_]))
        ++parsePosition_;
    UInt16 lineEnd=lineEnd_;
    while (lineEnd>parsePosition_ && isWhitespace(text_[lineEnd-1]))
        --lineEnd;
    IndentedParagraphElement* para=new IndentedParagraphElement();
    appendElement(para);
    pushParent(para);
    parseText(lineEnd, styleDefault);
    if (!lineAllowsContinuation(indentedLine))
        popParent();
}

//! @todo Implement DefinitionParser::parseDefinitionListLine()
void DefinitionParser::parseDefinitionListLine()
{
}