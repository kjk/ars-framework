#include <DefinitionParser.hpp>
#include <ListNumberElement.hpp>
#include <BulletElement.hpp>
#include <ParagraphElement.hpp>
#include <HorizontalLineElement.hpp>
#include <UTF8_Processor.hpp>

#ifdef INFOMAN
#include "HyperlinkHandler.hpp"
#endif

#include <memory>
#include <Utility.hpp>
#include <Text.hpp>
#include <LangNames.hpp>


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
    lastListNesting_(NULL),
    lastElementStart_(0),
    lastElementEnd_(0),
    unnamedLinksCount_(0),
    lineType_(emptyLine),
    previousLineType_(emptyLine),
    textPosition_(0),
    defaultLanguage("")
{
}    

void DefinitionParser::clear()
{
    textLine_.clear();
    parentsStack_.clear();
    currentNumberedList_.clear();
    numListsStack_.clear();
    if (NULL != lastListNesting_)
    {
        free(lastListNesting_);
        lastListNesting_ = NULL;
    }
    DestroyElements(elements_);
    elements_.clear();
}
    
DefinitionParser::~DefinitionParser()
{
    clear();
}

DefinitionElement* DefinitionParser::currentParent()
{
    if (!parentsStack_.empty())
        return parentsStack_.back();
    return NULL;
}

status_t DefinitionParser::pushParent(DefinitionElement* parent)
{
    status_t err = errNone;
	ErrTry {
		parentsStack_.push_back(parent);
	}
	ErrCatch(ex) {
		err = ex;
	} ErrEndCatch
	return err;
}

void DefinitionParser::popParent()
{
    assert(!parentsStack_.empty());
    parentsStack_.pop_back();
}

inline bool DefinitionParser::isPlainText() const
{
    return !(openEmphasize_ || openStrong_ || openVeryStrong_ || openTypewriter_ ||
        openSmall_ || openStrikeout_ || openUnderline_ || openSubscript_ || openSuperscript_);
}

status_t DefinitionParser::applyCurrentFormatting(TextElement* element)
{
    assert(NULL != element);
    DefinitionStyle* style = new_nt DefinitionStyle();
    if (NULL == style)
        return memErrNotEnoughSpace;

#ifdef _PALM_OS
    style->reset();
#endif

    const DefinitionStyle* def = StyleGetStaticStyle(styleIndexDefault);
    if (NULL != currentStyle_ && def != currentStyle_)
        *style |= *currentStyle_;
        
    element->setStyle(style, DefinitionElement::ownStyle);

    if (openEmphasize_)
        style->italic = style->yes;
        
#ifdef _PALM_OS
    if (openStrong_ || openVeryStrong_)
        style->bold = style->yes;
#else
	if (openStrong_)
		style->fontWeight = style->fontWeightBold;
	if (openVeryStrong_)
		style->fontWeight = style->fontWeightBlack;
#endif

    if (openStrikeout_)
        style->strike = style->yes;
        
#ifdef _PALM_OS
    if (openUnderline_)
        style->underline = solidUnderline;
#else
    if (openUnderline_)
        style->underline = style->yes;
#endif
	        
    if (openSubscript_)
        style->subscript = style->yes;
        
    if (openSuperscript_)
        style->superscript = style->yes;

	if (openSmall_)
		style->small = style->yes;

#ifdef _WIN32
	
	if (this->openTypewriter_)
		style->fontFamily = style->fontFamilyMonospace;

#endif

	return errNone;
}

/*
#define entityReferenceStart _T('&')
#define entityReferenceEnd   _T(';')

void DefinitionParser::decodeHTMLCharacterEntityRefs(String& text) const
{
    uint_t length = text.length();
    uint_t index = 0;
    bool inEntity = false;
    uint_t entityStart = 0;

    while (index<length)
    {
        char_t chr = text[index];
        if (!inEntity && chr == entityReferenceStart)
        {
            inEntity = true;
            entityStart = index;
        }
        else if (inEntity)
        {
            if (chr == entityReferenceEnd)
            {
                String entity(text, entityStart + 1, index-entityStart-1);
                if (!entity.empty() && entity[0] == _T('#'))
                {
                    long numVal;
                    const char_t* begin = entity.c_str();
                    status_t err = numericValue(begin + 1, begin + entity.length()-1, numVal);
                    if (errNone != err || numVal<0 || numVal>255)
                        chr = 1;
                    else
                        chr = char_t(numVal);
                }
                else
                    chr = 1;
                if (chr)
                {
                    text.replace(entityStart, index-entityStart + 1, &chr, 1);
                    length = text.length();
                    index = entityStart;
                }
                inEntity = false;
            }
            else if (!(chr=='#' || isAlNum(chr)))
                inEntity = false;
        }
        ++index;
    }
}
 */

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

status_t DefinitionParser::detectStrongTag(ulong_t  end, bool& isStrongTag)
{
	status_t err = errNone;
    isStrongTag = false;
    if (startsWith(textLine_, emphasizeText, textPosition_))
    {
        err = createTextElement();
        if (errNone != err)
			return err;
			
        if (startsWith(textLine_, strongText, textPosition_))
        {
            isStrongTag = true;
            if (startsWith(textLine_, veryStrongText, textPosition_))
            {
                textPosition_ += Len(veryStrongText);
                openVeryStrong_ = !openVeryStrong_;
            }
            else
            {
                textPosition_ += Len(strongText);
                openStrong_ = !openStrong_;
            }
        }
        else
        {
            isStrongTag = true;
            textPosition_ += Len(emphasizeText);
            openEmphasize_ = !openEmphasize_;
        }
    }
    return errNone;
}

#define nowikiText "nowiki"
#define teleTypeText "tt"
#define lineBreakText "br"
#define smallText "small"
#define strikeOutText "strike"
#define underlineText "u"
#define subscriptText "sub"
#define superscriptText "sup"

status_t DefinitionParser::detectHTMLTag(ulong_t end, bool& result)
{
    result = false;
    ulong_t tagStart = textPosition_ + 1;
    bool isClosing = false;
    if (tagStart < end && htmlClosingTagChar == textLine_[tagStart])
    {
        ++tagStart;
        isClosing = true;
    }
    if (tagStart<end)
    {
        String::size_type tagEndPos = textLine_.find(htmlTagEnd, tagStart);
        ulong_t tagEnd = (tagEndPos == textLine_.npos)?end:tagEndPos;
        if (tagEnd<end)
        {
            if (startsWithIgnoreCase(textLine_, nowikiText, tagStart))
            {
                openNowiki_ += (isClosing?-1:1);
                result = true;
            }
            else if (0 == openNowiki_)
            {
                if (startsWithIgnoreCase(textLine_, teleTypeText, tagStart))
                {
                    openTypewriter_ += (isClosing?-1:1);
                    result = true;
                }
                else if (startsWithIgnoreCase(textLine_, lineBreakText, tagStart))
                {
                    appendElement(new_nt LineBreakElement());
                    result = true;
                }
                else if (startsWithIgnoreCase(textLine_, smallText, tagStart))
                {
                    openSmall_ += (isClosing?-1:1);
                    result = true;
                }
                else if (startsWithIgnoreCase(textLine_, strikeOutText, tagStart))
                {
                    openStrikeout_ += (isClosing?-1:1);
                    result = true;
                }
                else if (startsWithIgnoreCase(textLine_, underlineText, tagStart))
                {
                    openUnderline_ += (isClosing?-1:1);
                    result = true;
                }
                else if (startsWithIgnoreCase(textLine_, superscriptText, tagStart))
                {
                    openSuperscript_ += (isClosing?-1:1);
                    result = true;
                }
                else if (startsWithIgnoreCase(textLine_, subscriptText, tagStart))
                {
                    openSubscript_+= (isClosing?-1:1);
                    result = true;
                }
            }
            if (result)
            {
				if (NULL == createTextElement())
					return memErrNotEnoughSpace;
				
                textPosition_ = tagEnd + 1;
            } 
        }
    }
    return errNone;
}

status_t DefinitionParser::detectHyperlink(ulong_t end, bool& result)
{
    result = false;
    bool hyperlinkIsTerm = false;
    status_t err = errNone;
    
    assert(linkOpenChar == textLine_[textPosition_] || linkCloseChar == textLine_[textPosition_]);
    if (!insideHyperlink_ && linkOpenChar == textLine_[textPosition_++])
    {
        err = createTextElement();
        if (errNone != err)
			return err;
			
        insideHyperlink_ = true;
        String::size_type separatorPos;
        if (linkOpenChar == textLine_[textPosition_])
        {
            hyperlinkIsTerm = true;
#ifdef INFOMAN
            hyperlinkType_ = hyperlinkUrl;
#else 
            hyperlinkType_ = hyperlinkTerm;
#endif            
            ++textPosition_;
            separatorPos = textLine_.find_first_of("|]", textPosition_);
        }
        else
        {
#ifdef INFOMAN
            hyperlinkType_ = hyperlinkUrl;
#else 
            hyperlinkType_ = hyperlinkExternal;
#endif            
            separatorPos = textLine_.find_first_of(" ]", textPosition_);
        }
        bool hasSeparator = true;
        if (textLine_.npos == separatorPos)
        {
            separatorPos = end;
            hasSeparator = false;
        }
        else if (linkCloseChar == textLine_[separatorPos])
            hasSeparator = false;
            
        hyperlinkTarget_.assign(textLine_, textPosition_, separatorPos - textPosition_);
        
#ifdef INFOMAN
        if (hyperlinkIsTerm)
        {
            if (String::npos == hyperlinkTarget_.find(urlSeparatorSchema))
            {
                hyperlinkTarget_.insert(0, 1, urlSeparatorSchema);
                hyperlinkTarget_.insert(0, defaultLanguage);
            }
            hyperlinkTarget_.insert(0, urlSchemaEncyclopediaTerm urlSeparatorSchemaStr);
        }
#endif                

        if (hasSeparator)
        {
            textPosition_ = separatorPos + 1;
            while (textPosition_<end && isSpace(textLine_[textPosition_]))
                ++textPosition_;
        }
        else
        {
            if (!hyperlinkIsTerm)
            {
                textPosition_ = separatorPos;
                ++unnamedLinksCount_;
                char buffer[8];

#if defined(_PALM_OS)
				StrPrintF(buffer, "[%lu]", unnamedLinksCount_);
#else
				sprintf(buffer, "[%lu]", unnamedLinksCount_);
#endif                
                err = createTextElement(buffer);
				if (errNone != err)
					return err;
           }
        }
        result = true;
    }
    else if (insideHyperlink_ && linkCloseChar == textLine_[textPosition_])
    {
        const ulong_t linkEndPos = textPosition_;
        while (textPosition_<end && linkCloseChar == textLine_[textPosition_])
            ++textPosition_;
        const uint_t pastLinkEnd = textPosition_;
        while (textPosition_<end && isAlNum(textLine_[textPosition_]))
            ++textPosition_;
        if (pastLinkEnd == textPosition_)
        {
            lastElementEnd_ = linkEndPos;
            while (lastElementEnd_>lastElementStart_ && isSpace(textLine_[lastElementEnd_-1]))
                --lastElementEnd_;
                
            err = createTextElement();
			if (errNone != err)
				return err;
        }
        else 
        {
            NarrowString text(textLine_, lastElementStart_, lastElementEnd_ - lastElementStart_);
            text.append(textLine_, pastLinkEnd, textPosition_ - pastLinkEnd);
            err = createTextElement(text, 0, text.length());
			if (errNone != err)
				return err;
        }
        insideHyperlink_ = false;
        result = true;
    }
    return errNone;
}

namespace {

    inline static bool isNewline(char chr)
    {
        return chr == '\n';
    }

}

status_t  DefinitionParser::parseText(ulong_t end, const DefinitionStyle* style)
{
	status_t err = errNone;
	
    openEmphasize_ = false;
    openStrong_ = false;
    openVeryStrong_ = false;
    openTypewriter_ = 0;
    openSmall_ = 0;
    openStrikeout_ = 0;
    openUnderline_ = 0;
    openSuperscript_ = 0;
    openSubscript_ = 0;
    currentStyle_ = style;
    hyperlinkTarget_.clear();
    insideHyperlink_ = false;

    if (end < parsePosition_)
        return errNone;

    ulong_t length = end - parsePosition_;
    while (length && isSpace((*text_)[parsePosition_ + length-1]))
        --length;
    textLine_.assign(*text_, parsePosition_, length);
    parsePosition_ = end;

    lastElementStart_ = textPosition_ = 0;
    while (textPosition_ < length)
    {
        assert(textPosition_ < textLine_.length());
        char chr = textLine_[textPosition_];
        if (isNewline(chr))
        {
            chr = ' ';
            textLine_[textPosition_] = ' ';
        }
            
        bool specialChar = false;
        lastElementEnd_ = textPosition_;
        if (htmlTagStart == chr)
        	err = detectHTMLTag(length, specialChar);
        if (0 == openNowiki_)
        {
			if (strongChar == chr)
				err = detectStrongTag(length, specialChar);
			else if (linkOpenChar == chr || linkCloseChar == chr)
				err = detectHyperlink(length, specialChar);
        }
        if (errNone != err)
			return err;
		
		if (specialChar)
			lastElementStart_ = textPosition_;
		else
            ++textPosition_;
    }
    lastElementEnd_ = textPosition_;
    return createTextElement();
}

status_t DefinitionParser::createTextElement(const char* text, long length, TextElement** elem)
{
	if (NULL != elem)
		*elem = NULL;
		
	if (-1 == length) 
		length = Len(text);
		
	if (0 == length)
		return errNone;
	
	status_t err = errNone;
	char* copy = NULL;
	
    bool hyperlinkIsTerm = false;
    
#ifdef INFOMAN
    ulong_t schemaLen = Len(urlSchemaEncyclopediaTerm) + 1;
    if (insideHyperlink_ && 0 == hyperlinkTarget_.find(urlSchemaEncyclopediaTerm urlSeparatorSchemaStr))
        hyperlinkIsTerm = true;
#else
    if (hyperlinkTerm == hyperlinkType_)
        hyperlinkIsTerm = true;
#endif    

    const char_t* langName = NULL;
    const char* langCode = NULL;
    ulong_t langCodeLen = 0;
    if (insideHyperlink_ && hyperlinkIsTerm)
    {
#ifdef INFOMAN        
        ulong_t startPos = schemaLen;
#else
        ulong_t startPos = 0;
#endif
        if (startPos == hyperlinkTarget_.find(text, startPos))
        {
            long colonPos = StrFind(text, length, ':');
            // This is to remove possible lang: part from hyperlink caption.
            if (-1 != colonPos)
            {
                langName = GetLangNameByLangCode(text, colonPos); 
                if (NULL != langName)
                {
					length -= colonPos + 1; 
					copy = StringCopy2N(text + colonPos + 1, length);
					if (NULL == copy)
						return memErrNotEnoughSpace;

					text = copy;						
                    langCode = hyperlinkTarget_.data() + startPos;
                    langCodeLen = colonPos;
                }
            }
        }
        if (NULL == langName)
        {
            ulong_t colonPos = hyperlinkTarget_.find(':', startPos);
            if (NarrowString::npos != colonPos)
            {
                langName = GetLangNameByLangCode(hyperlinkTarget_.data() + startPos, colonPos - startPos);
                if (NULL != langName)
                {
                    langCode = hyperlinkTarget_.data() + startPos;
                    langCodeLen = colonPos - startPos;
                }
            }
        }
    }
	
	ulong_t encLen = 0;
	char_t* enc;
	ulong_t len = length;
	
	err = UTF8_ToNative(text, len, enc, encLen);
	free(copy);
	if (errNone != err)
		return err;
			
    if (NULL != langCode && !StrEquals(langCode, langCodeLen, defaultLanguage))
	{
		enc = StrAppend(enc, encLen, _T(" ("), -1);
		if (NULL == enc)
			return memErrNotEnoughSpace;
		enc = StrAppend(enc, -1, langName, -1);
		if (NULL == enc)
			return memErrNotEnoughSpace;
		enc = StrAppend(enc, -1, _T(")"), -1);
		if (NULL == enc)
			return memErrNotEnoughSpace;		
	} 
    err = appendElement(new_nt TextElement());
	if (errNone != err)
	{	
		free(enc);
		return err;
	}
	TextElement* te = (TextElement*)elements_.back();
	err = te->setText(enc);
	free(enc);
	if (errNone != err)
		return err;
		 
    if (!isPlainText())
	{ 
		err = applyCurrentFormatting(te);
		if (errNone != err)
			return err;
	}
    else
        te->setStyle(currentStyle_);
    
    if (insideHyperlink_)
	{
		err = te->setHyperlink(hyperlinkTarget_.data(), hyperlinkTarget_.length(), hyperlinkType_);
		if (errNone != err)
			return err;
	}

    if (NULL != elem)
		*elem = te;
	
	return errNone; 
}


status_t DefinitionParser::createTextElement(TextElement** elem)
{
	if (NULL != elem)
		*elem = NULL;
		
    if (lastElementStart_ < lastElementEnd_)
        return createTextElement(textLine_, lastElementStart_, lastElementEnd_ - lastElementStart_, elem);
        
    return errNone;
}

status_t DefinitionParser::createTextElement(const NarrowString& text, ulong_t start, ulong_t len, TextElement** elem)
{
	if (NULL != elem)
		*elem = NULL;
	
	if (NarrowString::npos == len || len > text.length() - start)
		len = text.length() - start;
	
	if (0 != len)	
		return createTextElement(text.data() + start, long(len), elem);
	
	return errNone;
}


status_t DefinitionParser::startNewNumberedList(ListNumberElement* firstElement)
{
    if (currentNumberedList_.empty())
    {
        assert(numListsStack_.empty());
        return appendListNumber(firstElement);
    }
    else 
    {
		// TODO: redesign to catch errors 
        numListsStack_.push_back(NumberedList_t(1, firstElement));
        currentNumberedList_.swap(numListsStack_.back());
        return errNone;
    }
}

void DefinitionParser::finishCurrentNumberedList()
{
    assert(!currentNumberedList_.empty());
    uint_t totalCount = currentNumberedList_.size();
    for (uint_t i = 0; i<totalCount; ++i)
        currentNumberedList_[i]->setTotalCount(totalCount);
    if (!numListsStack_.empty())
    {
        currentNumberedList_.swap(numListsStack_.back());
        numListsStack_.pop_back();
    }
    else
        currentNumberedList_.clear();
}

#define maxNestingDepth 8

status_t DefinitionParser::manageListNesting(const char* requestedNesting)
{
    uint_t newNestingDepth = Len(requestedNesting);

    if (newNestingDepth > maxNestingDepth)
        newNestingDepth = maxNestingDepth;

    uint_t lastNestingDepth = 0;
    if (NULL != lastListNesting_)
        lastNestingDepth = Len(lastListNesting_);

    if ( (0 == lastNestingDepth) && (0 == newNestingDepth) )
        return errNone;

    char* newNesting = StringCopy2N(requestedNesting, newNestingDepth);
    if (NULL == newNesting)
        return memErrNotEnoughSpace;

    uint_t firstDiff = 0;  // This will be index of first character that makes previous and current nesting descr. differ.
    while (firstDiff < std::min(lastNestingDepth, newNestingDepth) && 
        lastListNesting_[firstDiff]==newNesting[firstDiff])
    {
        firstDiff++;
    }
        
    if (lastNestingDepth > 0)
    {
        for (uint_t i = lastNestingDepth; i>firstDiff; --i)
        {
            char listType = lastListNesting_[i-1];
            if (numberedListChar == listType)
                finishCurrentNumberedList();
            popParent();
        }
    }
    
    if (newNestingDepth > 0)
    {
        bool continueList = false;
        if (firstDiff == newNestingDepth) // Means we have just finished a sublist and next element will be another point in current list, not a sublist
        {
            assert(firstDiff > 0); 
            popParent();  
            --firstDiff;                
            continueList = true;    // Mark that next created element should be continuation of existing list, not start of new one
        }
        for (uint_t i = firstDiff; i < newNestingDepth; ++i)
        {
            char elementType = newNesting[i];
            status_t err = errNone;
            if (numberedListChar == elementType)
            {
				uint_t index = 1;
                if (continueList)
                {
                    assert(!currentNumberedList_.empty());
                    index = currentNumberedList_.back()->number() + 1;
                }
                err = appendElement(new_nt ListNumberElement(index));
				if (errNone != err)
				{
					free(newNesting);
					return err;
				}
				
				if (continueList)
					err = appendListNumber((ListNumberElement*)elements_.back()); // currentNumberedList_.push_back((ListNumberElement*)elements_.back());
                else
                    err = startNewNumberedList((ListNumberElement*)elements_.back());
            }
            else if (bulletChar == elementType)
                err = appendElement(new_nt BulletElement());
            else 
                err = appendElement(new_nt ParagraphElement(true));
                
            if (errNone != err)
			{
				free(newNesting);
				return err;
			}
            pushParent(elements_.back());
            continueList = false;
        }
    }

    if (NULL != lastListNesting_)
        free(lastListNesting_);
    lastListNesting_ = newNesting;
	return errNone;
}

status_t DefinitionParser::appendElement(DefinitionElement* element)
{
	if (NULL == element)
		return memErrNotEnoughSpace;
		
    element->setParent(currentParent());
       status_t err = errNone;
	ErrTry {
		elements_.push_back(element);
	}
	ErrCatch(ex) {
		err = ex;
	} ErrEndCatch
	if (errNone != err)
	{
		delete element;
		return err;
	}
	return errNone;
}

status_t DefinitionParser::appendListNumber(ListNumberElement* element)
{
	if (NULL == element)
		return memErrNotEnoughSpace;
		
	status_t err = errNone;
	ErrTry {
		currentNumberedList_.push_back(element);
	}
	ErrCatch(ex) {
		err = ex;
	} ErrEndCatch
	if (errNone != err)
	{
		delete element;
		return err;
	}
	return errNone;
}


DefinitionParser::LineType DefinitionParser::detectLineType(ulong_t start, ulong_t end) const
{
    LineType lineType = textLine;
    if (0 == openNowiki_)
    {
        if (end == start || (end - start == 1 && (*text_)[start]== '\r'))
            lineType = emptyLine;
        else {
            switch ((*text_)[start])
            {
                case indentLineChar:
                case bulletChar:
                case numberedListChar:
                    lineType = listElementLine;
                    break;
                
                case definitionListChar:
                    lineType = definitionListLine;
                    break;
                
                case headerChar:
                    if (startsWith(*text_, sectionString, start))
                        lineType = headerLine;
                    break;
                    
                case horizontalLineChar:
                    if (startsWith(*text_, horizontalLineString, start))
                        lineType = horizontalBreakLine;
                    break;
            }       
        }
    }
    return lineType;
}

bool DefinitionParser::detectNextLine(ulong_t textEnd, bool finish)
{
    String::size_type end = text_->find('\n', parsePosition_);
    bool goOn=(text_->npos != end && end < textEnd);
    if (finish || goOn)
    {
        LineType previousLineType = lineType_;
        ulong_t lineEnd=((text_->npos == end || end>=textEnd)?textEnd:end);
        LineType lineType = detectLineType(parsePosition_, lineEnd);
        if (textLine == lineType)
        {
            goOn = false;
            while (lineEnd + 1 < textEnd || finish)
            {
                end = text_->find('\n', lineEnd + 1);
                if (!finish && (text_->npos == end || end >= textEnd))
                    break;
                if (finish && lineEnd == textEnd)
                    goto LineFinished;
                ulong_t nextLineEnd=((text_->npos == end || end >= textEnd)?textEnd:end);
                if (textLine == detectLineType(lineEnd + 1, nextLineEnd))
                    lineEnd = nextLineEnd;
                else
                {
                    goOn = true;
                    goto LineFinished;
                }
            }
        }
        else
        {
LineFinished:        
            previousLineType_ = previousLineType;
            lineEnd_ = lineEnd;
            lineType_ = lineType;
        }
    }
    return goOn;
}


status_t DefinitionParser::parseTextLine()
{
	status_t err = errNone;
    if (!lineAllowsContinuation(previousLineType_) )
    {
        err = appendElement(new_nt ParagraphElement());
        if (errNone != err)
			return err;
			
        err = pushParent(elements_.back());
        if (errNone != err)
			return err;
    }
    return parseText(lineEnd_, StyleGetStaticStyle(styleIndexDefault));                
}

status_t DefinitionParser::handleIncrement(const char* text, ulong_t& length, bool finish)
{
     volatile status_t err = errNone;
     ErrTry {
		NarrowString strText(text, length);
        text_ = &strText;
        parsePosition_ = 0;
        lineEnd_ = 0;
        bool goOn = false;
        do 
        {
#ifndef NDEBUG    
            const char* text = text_->data() + parsePosition_;
#endif        
            goOn = detectNextLine(length, finish);
            if (goOn || finish)
            {
                if (lineAllowsContinuation(previousLineType_) && textLine != lineType_)
                    popParent(); 
                    
                if (listElementLine == previousLineType_ && listElementLine != lineType_)
				{ 
                    err = manageListNesting("");
                    if (errNone != err)
                        goto Finish;
                }

                switch (lineType_)
                {
                    case headerLine:
                        err = parseHeaderLine();
                        break;
                        
                    case textLine:
                        err = parseTextLine();
                        break;                    
                    
                    case horizontalBreakLine:
                        err = appendElement(new_nt HorizontalLineElement());
                        break;
                        
                    case emptyLine:
                        err = appendElement(new_nt LineBreakElement());
                        break;
                        
                    case listElementLine:
                        err = parseListElementLine();
                        break;
                        
                    case definitionListLine:
                        err = parseDefinitionListLine();
                        break;
                        
                    default:
                        assert(false);        
                }
				if (errNone != err)
                    goto Finish;
					 
                parsePosition_ = lineEnd_ + 1;
            }
        } while (goOn);
        if (finish && emptyLine != lineType_)
        {
            if (lineAllowsContinuation(previousLineType_))
                popParent(); 
                
            if (listElementLine == previousLineType_)
            {
                err = manageListNesting("");
				if (errNone != err)
                            goto Finish;
			} 
            
            assert(numListsStack_.empty());
            assert(currentNumberedList_.empty());            
        }
Finish:
        if (!finish)
            length = parsePosition_;
    }
     ErrCatch (ex) {
        err = ex;
    } ErrEndCatch
    if (errNone != err)
        clear();
    return err;
}

//! @todo Add header indexing
status_t DefinitionParser::parseHeaderLine()
{
    while (parsePosition_<lineEnd_ && (headerChar==(*text_)[parsePosition_] || isSpace((*text_)[parsePosition_])))
        ++parsePosition_;
        
    ulong_t lineEnd = lineEnd_;
    while (lineEnd>parsePosition_ && (headerChar==(*text_)[lineEnd-1] || isSpace((*text_)[lineEnd-1])))
        --lineEnd;
        
    status_t err = appendElement(new_nt ParagraphElement());
	if (errNone != err)
		return err;
		
    err = pushParent(elements_.back());
	if (errNone != err)
		return err;
		 
    err = parseText(lineEnd, StyleGetStaticStyle(styleNameHeader));
	if (errNone != err)
		return err;

    if (!lineAllowsContinuation(headerLine))
        popParent();
        
    return errNone;
}

status_t DefinitionParser::parseListElementLine()
{
    String::size_type startLong = text_->find_first_not_of(listCharacters, parsePosition_);
    ulong_t start = lineEnd_;
    if (text_->npos != startLong && startLong < lineEnd_)
        start = startLong;

    ulong_t elementDescLen = start - parsePosition_;
    char* elementDesc = StringCopy2N(text_->data() + parsePosition_, elementDescLen);
    if (NULL == elementDesc)
		return memErrNotEnoughSpace;
		 
    status_t err = manageListNesting(elementDesc);
    free(elementDesc);
	if (errNone != err)
		return err; 

    parsePosition_ = start;
    while (parsePosition_ < lineEnd_ && isSpace((*text_)[parsePosition_]))
        ++parsePosition_;
        
    return parseText(lineEnd_, StyleGetStaticStyle(styleIndexDefault));
}

status_t DefinitionParser::parseDefinitionListLine()
{
    while (parsePosition_ < lineEnd_ && (definitionListChar==(*text_)[parsePosition_] || isSpace((*text_)[parsePosition_])))
        ++parsePosition_;
        
    ulong_t lineEnd = lineEnd_;
    while (lineEnd > parsePosition_ && isSpace((*text_)[lineEnd - 1]))
        --lineEnd;
        
    status_t err = appendElement(new_nt ParagraphElement());
	if (errNone != err)
		return err;
		 
    err = pushParent(elements_.back());
	if (errNone != err)
		return err;

    err = parseText(lineEnd, StyleGetStaticStyle(styleIndexDefault));
	if (errNone != err)
		return err;

    if (!lineAllowsContinuation(definitionListLine))
        popParent();
	
	return errNone;
}

DefinitionModel* DefinitionParser::createModel()
{
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
    
   elements_.swap(model->elements);
   return model;
}
