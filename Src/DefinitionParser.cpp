#include "DefinitionParser.hpp"
#include "FormattedTextElement.hpp"
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

void DefinitionParser::decode(ArsLexis::String& text) const
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

