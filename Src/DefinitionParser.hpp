#ifndef __DEFINITION_PARSER_HPP__
#define __DEFINITION_PARSER_HPP__

#include "Debug.hpp"
#include "RenderingPreferences.hpp"
#include "Definition.hpp"
#include "HTMLCharacterEntityReferenceDecoder.hpp"
#include <list>
#include <vector>

class Definition;

class DefinitionElement;

class FormattedTextElement;

class DefinitionParser
{
    Boolean openEmphasize_;
    Boolean openStrong_;
    Boolean openVeryStrong_;
    UInt16 openTypewriter_;
    UInt16 openSmall_;
    UInt16 openStrikeout_;
    UInt16 openUnderline_;
    UInt16 openNowiki_;
    
    void reset();
    
    Boolean isPlainText() const;
    
    void applyCurrentFormatting(FormattedTextElement* element);
    
    typedef std::list<DefinitionElement*, ArsLexis::Allocator<DefinitionElement*> > ParentsStack_t;
    ParentsStack_t parentsStack_;
    
    DefinitionElement* currentParent();
    
    void pushParent(DefinitionElement* parent)
    {parentsStack_.push_back(parent);}
    
    void popParent()
    {parentsStack_.pop_back();}
    
    struct ParentSetter;
    friend struct ParentSetter;
    struct ParentSetter
    {
        DefinitionParser& parser;
        ParentSetter(DefinitionParser& p, DefinitionElement* parent):
            parser(p)
        {
            parser.pushParent(parent);
        }
        
        ~ParentSetter()
        {
            parser.popParent();
        }
    };
    
    Definition definition_;
    ArsLexis::String text_;
    
    Err subParseText(UInt16 n, ElementStyle style);
    
    Err parse();
    
    ArsLexis::HTMLCharacterEntityReferenceDecoder decoder_;

    void decode(ArsLexis::String& text) const;
    
    UInt16 parsePosition_;
    
    enum InlineRule 
    {
        ruleEmphasize,
        ruleStrong,
        ruleVeryStrong,
        ruleTypewriterStart,
        ruleTypewriterEnd,
        ruleSmallStart,
        ruleSmallEnd,
        ruleStrikeoutStart,
        ruleStrikeoutEnd,
        ruleUnderlineStart,
        ruleUnderlineEnd,
        ruleNowikiStart,
        ruleNowikiEnd,
        ruleSequenceEnd
    };
    
public:

    DefinitionParser();
    
    Err parse(const ArsLexis::String& text, Definition& definition);
    
    ~DefinitionParser();

};

#endif