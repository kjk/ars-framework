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

class GenericTextElement;

class FormattedTextElement;

class ListNumberElement;

/**
 * Transforms textual definition representation into Definition object.
 * Parser should be initiaited with reference to @c String that acts as a response
 * buffer. After each update to buffer's content @c parseIncrement() should be called.
 * When all data is received @c parseIncrement(true) should be called to parse all the
 * remeining lines.
 */
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
    
    /**
     * @internal
     * Transforms current settings in open* variables into settings of @c FormattedTextElement.
     */
    void applyCurrentFormatting(FormattedTextElement* element);
    
    typedef std::list<DefinitionElement*, ArsLexis::Allocator<DefinitionElement*> > ParentsStack_t;
    ParentsStack_t parentsStack_;
    
    DefinitionElement* currentParent();
    
    void pushParent(DefinitionElement* parent);
    
    void popParent();
    
    void popAvailableParent();

    /**
     * @internal
     * Holds text that specifies what was nesting level of last parsed list element 
     * in the standard form (the "*#" texts). This way we know whether currently
     * parsed list element is continuation of previous list or start of a new one.
     * Therefore this variable should be cleared when we get outside of any list.
     */
    ArsLexis::String lastListNesting_;

    /**
     * @internal
     * For purpose of setting ListNumberElement's totalCount member we need collection
     * that has cheap size() implementation, and list typically doesn't. 
     */
    typedef std::vector<ListNumberElement*, ArsLexis::Allocator<ListNumberElement*> > NumberedList_t;
    typedef std::list<NumberedList_t, ArsLexis::Allocator<NumberedList_t> > NumberedListsStack_t;
    
    /**
     * @internal
     * 
     */
    NumberedListsStack_t numListsStack_;
    NumberedList_t currentNumberedList_;
    
    /**
     * @internal 
     * Pushes on stack current numbered list (if there is one) and starts a new one 
     * with @c firstElement.
     */
    void startNewNumberedList(ListNumberElement* firstElement);
    
    /**
     * @internal 
     * Sets totalCount of elements of current numbered list, and pops previous one
     * in its place from the stack (if there is one).
     */
    void finishCurrentNumberedList();

    /**
     * @internal 
     * Compares @c lastListNesting_ with @c newNesting and finishes/creates all
     * lists that stand in a way from previous to current one (this involves 
     * popping/pushing parents from/to parents stack). Creates appropriate
     * ListNumberElement or BulletElement afterwards and pushes it as current parent
     * (if newNesting isn't empty, which means that we want simply to finish all currently
     * opened lists and parse non-list element.
     */
    void manageListNesting(const ArsLexis::String& newNesting);
    
    Definition definition_;
    const ArsLexis::String& text_;
    UInt16 parsePosition_;
    UInt16 lineEnd_;
    UInt16 lastElementStart_;
    UInt16 lastElementEnd_;
    UInt16 unnamedLinksCount_;
    
    void parseText(UInt16 end, ElementStyle style);
    
    void parse();
    
    ArsLexis::HTMLCharacterEntityReferenceDecoder decoder_;

    void decodeHTMLCharacterEntityRefs(ArsLexis::String& text) const;
    
    void appendElement(DefinitionElement* element);
    
    enum LineType
    {
        emptyLine,
        headerLine,
        listElementLine,
        textLine,
        horizontalBreakLine,
        indentedLine,
        definitionListLine
    };
    
    LineType lineType_;
    LineType previousLineType_;

    ElementStyle currentStyle_;
    
    static Boolean lineAllowsContinuation(LineType lineType)
    {
        return listElementLine==lineType || textLine==lineType || indentedLine==lineType;
    }


    void parseHeaderLine();
    
    void parseIndentedLine();
    
    void parseDefinitionListLine();
    
    void parseTextLine();
    
    void parseListElementLine();
    
    bool detectNextLine(UInt16 end, bool finish);
    
    Boolean detectHTMLTag(UInt16 textEnd);
    
    Boolean detectStrongTag(UInt16 textEnd);
    
    Boolean detectHyperlink(UInt16 textEnd);
    
    GenericTextElement* createTextElement();
    
public:

    DefinitionParser(const ArsLexis::String& text, UInt16 initialOffset=0);

    void parseIncrement(UInt16 end, bool finish=false);
    
    void updateDefinition(Definition& definition);
    
    ~DefinitionParser();

};

#endif