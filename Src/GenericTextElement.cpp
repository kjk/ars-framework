#include "GenericTextElement.hpp"
#include "Utility.hpp"

using ArsLexis::String;
using ArsLexis::Rectangle;

GenericTextElement::~GenericTextElement()
{}

#define whitespaceChars " \t\n\r"

#ifdef NDEBUG
inline 
#endif
static UInt16 findNextWhitespace(const String& text, UInt16 fromPos)
{
    String::size_type nextWhitespace=text.find_first_of(whitespaceChars, fromPos);
    if (String::npos==nextWhitespace)
        nextWhitespace=text.length();
    return nextWhitespace;
}

void GenericTextElement::calculateOrRender(LayoutContext& layoutContext, Coord left, Coord top, Definition* definition, Boolean render)
{
    assert(!layoutContext.isElementCompleted());

    Coord baseLine=FntBaseLine();
    Coord lineHeight=FntLineHeight();
    const char* text=text_.c_str()+layoutContext.renderingProgress;
    left+=layoutContext.usedWidth;
    top+=(layoutContext.baseLine-baseLine);

    UInt16 nextWhitespace=findNextWhitespace(text_, layoutContext.renderingProgress)-layoutContext.renderingProgress;
    Int16 length=FntWordWrap(text, layoutContext.availableWidth());
    if (0==layoutContext.renderingProgress && !layoutContext.isFirstInLine() && length<nextWhitespace)
    {
        Int16 newLineLength=FntWordWrap(text, layoutContext.screenWidth);
        if (nextWhitespace<=newLineLength)
        {
            layoutContext.usedWidth+=layoutContext.availableWidth();
            return;
        }
    }

    if (!render)
        layoutContext.extendHeight(lineHeight, baseLine);

    Coord width=FntCharsWidth(text, length);

    if (render)
    {
        WinDrawChars(text, length, left, top);
        defineHotSpot(*definition, Rectangle(left, top, lineHeight, width));
    }

    left+=width;    
    text+=length;

    if (*text)
    {
        layoutContext.renderingProgress+=length;
        layoutContext.usedWidth+=width;
        nextWhitespace=findNextWhitespace(text_, layoutContext.renderingProgress)-layoutContext.renderingProgress;
        length=FntWordWrap(text, layoutContext.screenWidth);
        if (length<nextWhitespace)
        {
            Int16 newLineLength=FntWordWrap(text, layoutContext.screenWidth);
            if (newLineLength<nextWhitespace)
            {
                length=StrLen(text);
                width=layoutContext.availableWidth();
                Boolean notTruncated=false;
                FntCharsInWidth(text, &width, &length, &notTruncated);

                if (render)
                {
                    WinDrawChars(text, length, left, top);
                    defineHotSpot(*definition, Rectangle(left, top, lineHeight, width));
                }
                
                layoutContext.renderingProgress+=length;
                layoutContext.usedWidth+=layoutContext.availableWidth();
            }
        }
    }
    else
        layoutContext.markElementCompleted(width);    
}

void GenericTextElement::calculateLayout(LayoutContext& layoutContext)
{
    WinPushDrawState();
    prepareDrawState();
    calculateOrRender(layoutContext, 0, 0);
    WinPopDrawState();
}

void GenericTextElement::render(RenderingContext& renderContext)
{
    WinPushDrawState();
    prepareDrawState();
    calculateOrRender(renderContext, renderContext.left, renderContext.top, &renderContext.definition, true);
    WinPopDrawState();
}