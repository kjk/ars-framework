#include "DefinitionParser.hpp"
#include "FormattedTextElement.hpp"
#include "ListNumberElement.hpp"
#include "BulletElement.hpp"
#include "ParagraphElement.hpp"
#include "HorizontalLineElement.hpp"
#include <cctype>
#include "Utility.hpp"

using ArsLexis::String;

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
}
    
    
void DefinitionParser::parse(const String& text, Definition& definition)
{
    reset();
    text_=text;
    parse();
    definition.swap(definition_);
    definition_.clear();
    text_.clear();
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
                chr=decoder_.decode(entity);
                if (chr)
                {
                    text.replace(entityStart, index-entityStart+1, &chr, 1);
                    length=text.length();
                    index=entityStart;
                }
                inEntity=false;
            }
            else if (!std::isalnum(chr))
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

#define horizontalLineString "----"
#define sectionString "=="
#define subSectionString "==="
#define subSubSectionString "===="
#define listCharacters "*#"



void DefinitionParser::parseText(UInt16 length, ElementStyle style)
{
    

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
        currentNumberedList_.empty();
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
            
        
        for (UInt16 i=lastNestingDepth-1; i>=firstDiff; --i)
        {
            popParent();
            char listType=lastListNesting_[firstDiff];
            if (numberedListChar==listType)
                finishCurrentNumberedList();
        }

        Boolean continueList=false;
        if (firstDiff==newNestingDepth) // Means we have just finished a sublist and next element will be another point in current list, not a sublist
        {
            assert(firstDiff>0);   
            popParent();            // We need to pop previous sibling from parents stack
            --firstDiff;                
            continueList=true;    // Mark that next created element should be continuation of existing list, not start of new one
        }
        for (UInt16 i=firstDiff; i<newNestingDepth; ++i)
        {
            char elementType=newNesting[firstDiff-1];
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
    if (!continuationAllowed())
    {
        ParagraphElement* para=new ParagraphElement();
        appendElement(para);
        pushParent(para);
    }
    parseText(lineEnd_-parsePosition_, styleDefault);                
}

void DefinitionParser::parse()
{
    previousLineType_=lineType_;
    while (parsePosition_<text_.length())
    {
        detectLineType();

        if (continuationAllowed() && textLine!=lineType_)
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
    using ArsLexis::isWhitespace;
    while (parsePosition_<lineEnd_ && headerChar==text_[parsePosition_] || isWhitespace(text_[parsePosition_]))
        ++parsePosition_;
    UInt16 lineEnd=lineEnd_;
    while (lineEnd>parsePosition_ && headerChar==text_[lineEnd-1] || isWhitespace(text_[lineEnd-1]))
        --lineEnd;
    ParagraphElement* para=new ParagraphElement();
    appendElement(para);
    pushParent(para);
    parseText(lineEnd-parsePosition_, styleHeader);
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
    parseText(lineEnd_-parsePosition_, styleDefault);
}

void DefinitionParser::parseIndentedLine()
{
    
}