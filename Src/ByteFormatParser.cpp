#include "ByteFormatParser.hpp"

#include <Text.hpp>

#include <GenericTextElement.hpp>
#include <LineBreakElement.hpp>
#include <HorizontalLineElement.hpp>
#include <ParagraphElement.hpp>
#include <ListNumberElement.hpp>
#include <FormattedTextElement.hpp>
#include <BulletElement.hpp>

//TODO: fontEffects
//TODO: styles - now only 'b' for boldBlack is supported

//elements
#define typeLineBreakElement                'lbrk'
#define typeHorizontalLineElement           'hrzl'
#define typeGenericTextElement              'gtxt'
#define typeBulletElement                   'bull'
#define typeFormattedTextElement            'ftxt'
#define typeListNumberElement               'linu'
#define typeParagraphElement                'parg'
#define typeIndentedParagraphElement        'ipar'
//pop parent element
#define typePopParentElement                'popp'

//params
#define paramTextValue                      'text'
#define paramJustification                  'just'
#define paramStyle                          'styl'
#define paramHyperlink                      'hypl'
#define paramListNumber                     'lnum'
#define paramListTotalCount                 'ltcn'
#define paramLineBreakSize                  'muld'
//TODO: make it works
#define paramFontEffects                    'fnte'


//other (numbers)
#define typeLength  sizeof(elementTypeType)
#define sizeLength  sizeof(elementSizeType)
#define headerLength 12

using namespace ArsLexis;

ByteFormatParser::ByteFormatParser():
    start_(0),
    inLength_(0),
    finish_(false),
    currentElementType_(0),
    currentElementParamsLength_(0),
    currentParamType_(0),
    currentParamLength_(0),
    totalElementsCount_(0),
    elementsCount_(0),
    version_(0),
    totalSize_(0),
    headerParsed_(false)
{
    inText_.clear();
    stack_.clear();
}

ByteFormatParser::~ByteFormatParser()
{
    std::for_each(elems_.begin(), elems_.end(), ObjectDeleter<DefinitionElement>());
}

void ByteFormatParser::reset()
{
    std::for_each(elems_.begin(), elems_.end(), ObjectDeleter<DefinitionElement>());
    elems_.clear();
    stack_.clear();
    start_ = 0;
    inLength_ = 0;
    finish_ = false;
    inText_.clear();
    currentElementType_= 0;
    currentElementParamsLength_ = 0;
    currentParamType_ = 0;
    currentParamLength_ = 0;
    totalElementsCount_ = 0;
    elementsCount_ = 0;
    version_ = 0;
    totalSize_ = 0;
    headerParsed_ = false;
}

void ByteFormatParser::replaceElements(Definition::Elements_t& el)
{
    el.swap(elems_);
}

bool ByteFormatParser::parseParam()
{
    if (currentElementParamsLength_ == 0)
        return false;
    currentParamType_ = readUnaligned32(&inText_[start_]);
    start_ += typeLength;
    currentElementParamsLength_ -= typeLength;
    currentParamLength_ = 0;
    // calculate length
    currentElementParamsLength_ -= 1;
    switch(inText_[start_])
    {
        case _T('1'):
            start_++;
            currentParamLength_ = 1;
            break;
        case _T('2'):
            start_++;
            currentParamLength_ = 2;
            break;
        case _T('4'):
            start_++;
            currentParamLength_ = 4;
            break;
        case _T('8'):
            start_++;
            currentParamLength_ = 8;
            break;
        case _T('c'):
            start_++;
            currentParamLength_ = 12;
            break;

        default:
            assert(inText_[start_] == _T('L'));
            start_++;
            currentElementParamsLength_ -= sizeLength;
            currentParamLength_ = readUnaligned32(&inText_[start_]);
            start_ += sizeLength;
            break;
    }
    
    // do action (do not modify start_ or lengths)
    switch(currentParamType_)
    {
        case paramTextValue:
            {
                String str(inText_,start_,currentParamLength_);
                if (currentElementType_ == typeGenericTextElement ||
                    currentElementType_ == typeFormattedTextElement)
                    ((GenericTextElement*)currentElement_)->setText(str);
            }
            break;
            
        case paramListNumber:
            if (currentElementType_ == typeListNumberElement)
                ((ListNumberElement*)currentElement_)->setNumber(readUnaligned32(&inText_[start_]));
            break;
    
        case paramJustification:
            switch(inText_[start_])
            {
                case _T('l'):
                    currentElement_->setJustification(DefinitionElement::justifyLeft);
                    break;

                case _T('i'):
                    currentElement_->setJustification(DefinitionElement::justifyInherit);
                    break;

                case _T('c'):
                    currentElement_->setJustification(DefinitionElement::justifyCenter);
                    break;

                case _T('r'):
                    currentElement_->setJustification(DefinitionElement::justifyRight);
                    break;

                case _T('x'):
                    currentElement_->setJustification(DefinitionElement::justifyRightLastElementInLine);
                    break;
                
                default:
                    assert(false);
                    break;
            }
            break;

        case paramStyle:
            switch(inText_[start_])
            {
                case _T('b'):
                    currentElement_->setStyle(styleBoldBlack);
                    break;
                    
                case _T('g'):
                    currentElement_->setStyle(styleGray);
                    break;
                //TODO: add more styles?
            }
            break;

        case paramHyperlink:
            currentElement_->setHyperlink(String(inText_, start_, currentParamLength_), hyperlinkUrl);
            break;
            
        case paramListTotalCount:
            if (currentElementType_ == typeListNumberElement)
                ((ListNumberElement*)currentElement_)->setTotalCount(readUnaligned32(&inText_[start_]));
            break;

        case paramLineBreakSize:
            if (currentElementType_ == typeLineBreakElement)
            {
                ulong_t mul = readUnaligned32(&inText_[start_]);
                ulong_t div = readUnaligned32(&inText_[start_+4]);
                ((LineBreakElement*)currentElement_)->setSize(mul,div);
            }
            break;
            
        case paramFontEffects:
            if (currentElementType_ == typeFormattedTextElement)
            {
                ulong_t mask = readUnaligned32(&inText_[start_]);
                ulong_t fx = readUnaligned32(&inText_[start_+4]);
                FontEffects fontEffects;                
                //TODO: mask,fx -> fontEffects
                
                ((FormattedTextElement*)currentElement_)->setEffects(fontEffects);
            }
            break;
            
        default:
            // unsupported param
            assert(false);
            break;
    }
    // move start to next param
    start_ += currentParamLength_;
    currentElementParamsLength_ -= currentParamLength_;
    return true;    
}

void ByteFormatParser::parseElementParams()
{
    // first add element
    currentElement_ = NULL;
    switch(currentElementType_)
    {
        case typeLineBreakElement:
            elems_.push_back(currentElement_ = new LineBreakElement());
            break;

        case typeHorizontalLineElement:
            elems_.push_back(currentElement_ = new HorizontalLineElement());
            break;

        case typeGenericTextElement:
            elems_.push_back(currentElement_ = new GenericTextElement());
            break;

        case typeBulletElement:
            elems_.push_back(currentElement_ = new BulletElement());
            break;

        case typeFormattedTextElement:
            elems_.push_back(currentElement_ = new FormattedTextElement());
            break;

        case typeListNumberElement:
            elems_.push_back(currentElement_ = new ListNumberElement());
            break;

        case typeParagraphElement:
            elems_.push_back(currentElement_ = new ParagraphElement());
            break;

        case typeIndentedParagraphElement:
            elems_.push_back(currentElement_ = new IndentedParagraphElement());
            break;
            
        default:
            // we don't know what is this - better return
            assert(false);
            start_ += currentElementParamsLength_;
            return;
    }
    // set parent
    if (!stack_.empty())
        currentElement_->setParent(stack_.back());
    // format it...
    bool parsing = true;
    while (parsing)
    {
        parsing = parseParam();
        if (currentElementParamsLength_ == 0)
            parsing = false;
    }
    
    // add it to stack
    stack_.push_back(currentElement_);
}

/**
 *  return true if element is parsed (start_ is moved to next element)
 */
bool ByteFormatParser::parseElement()
{
    // can read type?
    if (inLength_ - start_ < typeLength)
        return false;
    currentElementType_ = readUnaligned32(&inText_[start_]);
    if (typePopParentElement == currentElementType_)
    {
        //no length, just pop from stack_...
        if (!stack_.empty())
            stack_.pop_back();
        else
            assert(false);
        start_ += typeLength;
        elementsCount_++;
        return true;
    }
    else
    {
        // can read totalSize?
        if (inLength_ - start_ < typeLength + sizeLength)
            return false; 
        currentElementParamsLength_ = readUnaligned32(&inText_[start_+typeLength]);
        // can read element params?
        if (inLength_ - start_ < typeLength + sizeLength + currentElementParamsLength_)
            return false;
        // so now we can read params and create element...
        start_ += typeLength + sizeLength;
        ArsLexis::String::size_type nextElementPos = start_ + currentElementParamsLength_;
        parseElementParams();
        assert(start_ == nextElementPos);
        // just to be sure!
        start_ = nextElementPos;
        elementsCount_++;
        return true;
    }
}


status_t ByteFormatParser::parse()
{
    while(parseElement())
    {
        //do nothing - just parsing
    }
    // remove parsed text from memory
    inText_.erase(0, start_);
    inLength_ -= start_;
    totalSize_ -= start_;
    start_ = 0;
    return errNone;
}

bool ByteFormatParser::parseHeader(const char_t* inText)
{
    if (headerParsed_)
        return true;
    
    totalSize_ = readUnaligned32(&inText[start_]);
    start_ += sizeLength;
    totalElementsCount_ = readUnaligned32(&inText[start_]);
    start_ += sizeLength;
    version_ = readUnaligned32(&inText[start_]);
    start_ += sizeLength;
    headerParsed_ = true;
    
    return true;
}
       
status_t ByteFormatParser::handleIncrement(const char_t* inputText, ulong_t inputLength, bool finish)
{
    inText_.append(inputText, inputLength);
    inLength_ += inputLength;
    start_ = 0;
    finish_ = finish;
    
    if (!headerParsed_)
    {
        if (inLength_ < headerLength)
            return errNone;
        parseHeader(inputText);    
    }
    
    if (inLength_ == 0)
        return errNone;
    status_t error = parse();
    if (inLength_ > 0 && finish_)
    {
        assert(false);
    }
    if (finish)
    {
        assert(elementsCount_ == totalElementsCount_);
        assert(inLength_ == totalSize_);
    }
    return error;
}

status_t ByteFormatParser::parseAll(const char_t* inputText, UInt32 inputTextLen)
{
    start_ = 0;

    parseHeader(inputText);
    assert(totalSize_ == inputTextLen || (UInt32)(-1) == inputTextLen);
    inText_.assign(inputText, totalSize_);
    inLength_ = totalSize_;
    finish_ = true;
    
    if (inLength_ == 0)
        return errNone;
    status_t error = parse();
    if (inLength_ > 0 && finish_)
    {
        assert(false);
    }
    assert(inLength_ == totalSize_);
    assert(elementsCount_ == totalElementsCount_);
    return error;
}

ulong_t readUnaligned32(const char* addr)
{
    return
    (
    (((ulong_t)((unsigned char)addr[0])) << 24) | 
    (((ulong_t)((unsigned char)addr[1])) << 16) | 
    (((ulong_t)((unsigned char)addr[2])) <<  8) | 
    ((ulong_t)((unsigned char)addr[3])) );
}    
void writeUnaligned32(char* addr, ulong_t value)
{
    addr[0] = (char)((ulong_t)(value) >> 24);
    addr[1] = (char)((ulong_t)(value) >> 16);
    addr[2] = (char)((ulong_t)(value) >>  8);
    addr[3] = (char)((ulong_t)(value));
}    

