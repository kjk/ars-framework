#include "DefinitionParser.hpp"
#include "FormattedTextElement.hpp"
#include "ListNumberElement.hpp"
#include "BulletElement.hpp"
#include <cctype>

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
    parsePosition_(0)
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
}
    
    
Err DefinitionParser::parse(const ArsLexis::String& text, Definition& definition)
{
    reset();
    text_=text;
    Err error=parse();
    if (!error)
        definition.swap(definition_);
    definition_.clear();
    text_.clear();
    return error;        
}

static const char entityReferenceStart='&';
static const char entityReferenceEnd=';';

void DefinitionParser::decodeHTMLCharacterEntityRefs(ArsLexis::String& text) const
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

static const char indentLineStart=':';
static const char bulletChar='*';
static const char numberedListChar='#';
static const char headerChar='=';
static const char strongChar='\'';
static const char htmlTagStart='<';

#define horizontalLineString "----"
#define sectionString "=="
#define subSectionString "==="
#define subSubSectionString "===="



Err DefinitionParser::subParseText(UInt16 n, ElementStyle style)
{
    

    return errNone;
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

void DefinitionParser::manageListNesting(const ArsLexis::String& newNesting)
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

