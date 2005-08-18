#include <ByteFormatParser.hpp>

#include <Text.hpp>

#include <TextElement.hpp>
#include <LineBreakElement.hpp>
#include <HorizontalLineElement.hpp>
#include <ParagraphElement.hpp>
#include <ListNumberElement.hpp>
#include <BulletElement.hpp>
#include <UTF8_Processor.hpp>

//elements
#define typeLineBreakElement                'lbrk'
#define typeHorizontalLineElement           'hrzl'
#define typeTextElement              'gtxt'
#define typeBulletElement                   'bull'
#define typeListNumberElement               'linu'
#define typeParagraphElement                'parg'
#define typeIndentedParagraphElement        'ipar'
//styles table (params are styles entries)
#define typeStylesTableElement              'sttb'
//form title (use getTitle() to get it)
#define typeTitleElement                    'titl'
//pop parent element
#define typePopParentElement                'popp'

//params
#define paramTextValue                      'text'
#define paramJustification                  'just'
#define paramHyperlink                      'hypl'
#define paramListNumber                     'lnum'
#define paramListTotalCount                 'ltcn'
#define paramLineBreakSize                  'muld'
#define paramStyleName                      'stnm'
//param of StylesTable - one style entry
#define paramStyleEntry                     'sten'
#define paramStylesCount                    'stcn'

//other (numbers)
#define typeLength  sizeof(elementTypeType)
#define sizeLength  sizeof(elementSizeType)
#define headerLength 12

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
    model_(NULL),
    styleCount_(0),
    totalStyleCount_(0),
    styleNames_(NULL),
    headerParsed_(false),
	inText_(NULL)
{
}

ByteFormatParser::~ByteFormatParser()
{
    delete model_;
    for (ulong_t i = 0; i < totalStyleCount_; i++)
        free(styleNames_[i]);
    delete [] styleNames_;
	free(inText_); 
}

DefinitionModel* ByteFormatParser::releaseModel()
{
    DefinitionModel* model = model_;
    model_ = NULL;
    for (ulong_t i = 0; i < totalStyleCount_; i++)
        free(styleNames_[i]);
    delete [] styleNames_;
    totalStyleCount_ = 0;    
    styleNames_ = NULL;
    return model;
}


void ByteFormatParser::reset()
{
    stack_.clear();
    start_ = 0;
    inLength_ = 0;
    finish_ = false;
	free(inText_);
	inText_ = NULL; 
    currentElementType_= 0;
    currentElementParamsLength_ = 0;
    currentParamType_ = 0;
    currentParamLength_ = 0;
    totalElementsCount_ = 0;
    elementsCount_ = 0;
    version_ = 0;
    totalSize_ = 0;
    headerParsed_ = false;
    styleCount_ = 0;
    totalStyleCount_ = 0;

    delete model_;
    model_ = NULL;
    for (ulong_t i = 0; i < totalStyleCount_; i++)
        free(styleNames_[i]);
    delete [] styleNames_;
    totalStyleCount_ = 0;    
    styleNames_ = NULL;
}

bool ByteFormatParser::parseParam()
{
    if (currentElementParamsLength_ == 0)
        return false;
        
    currentParamType_ = readUnaligned32(inText_ + start_);
    start_ += typeLength;
    currentElementParamsLength_ -= typeLength;
    currentParamLength_ = 0;
    // calculate length
    currentElementParamsLength_ -= 1;
    switch(inText_[start_])
    {
        case '1':
            start_++;
            currentParamLength_ = 1;
            break;
        case '2':
            start_++;
            currentParamLength_ = 2;
            break;
        case '4':
            start_++;
            currentParamLength_ = 4;
            break;
        case '8':
            start_++;
            currentParamLength_ = 8;
            break;
        case 'c':
            start_++;
            currentParamLength_ = 12;
            break;

        default:
            assert(inText_[start_] == 'L');
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
				ulong_t len;
				char_t* str = UTF8_ToNative(inText_ + start_, currentParamLength_, &len);
				// TODO: change interface to return error code!
				if (NULL != str)
				{
					if (typeTextElement == currentElementType_)
						((TextElement*)currentElement_)->setText(String(str, len));
					else if (typeTitleElement == currentElementType_)
					{
						assert(NULL != model_);
						model_->setTitle(str, len);
					}
					free(str);
				}
            }
            break;
            
        case paramListNumber:
            if (typeListNumberElement == currentElementType_)
                ((ListNumberElement*)currentElement_)->setNumber(readUnaligned32(inText_ + start_));
            break;
    
        case paramJustification:
            switch(inText_[start_])
            {
                case 'l':
                    currentElement_->setJustification(DefinitionElement::justifyLeft);
                    break;

                case 'i':
                    currentElement_->setJustification(DefinitionElement::justifyInherit);
                    break;

                case 'c':
                    currentElement_->setJustification(DefinitionElement::justifyCenter);
                    break;

                case 'r':
                    currentElement_->setJustification(DefinitionElement::justifyRight);
                    break;

                case 'x':
                    currentElement_->setJustification(DefinitionElement::justifyRightLastElementInLine);
                    break;
                
                default:
                    assert(false);
                    break;
            }
            break;

        case paramHyperlink:
            currentElement_->setHyperlink(inText_ + start_, currentParamLength_, hyperlinkUrl);
            break;
            
        case paramListTotalCount:
            if (typeListNumberElement == currentElementType_)
                ((ListNumberElement*)currentElement_)->setTotalCount(readUnaligned32(inText_ + start_));
            break;

        case paramLineBreakSize:
            if (typeLineBreakElement == currentElementType_)
            {
                ulong_t mul = readUnaligned32(inText_ + start_);
                ulong_t div = readUnaligned32(inText_ + start_ + 4);
                ((LineBreakElement*)currentElement_)->setSize(mul,div);
            }
            break;
            
        case paramStyleEntry:
            if (typeStylesTableElement == currentElementType_)
            {
                assert(styleCount_ < totalStyleCount_);
                if (styleCount_ < totalStyleCount_)
                {
                    // <nameLength><name><value>
                    ulong_t nameLength = readUnaligned32(inText_ + start_);
                    styleNames_[styleCount_] = StringCopyN(inText_ + start_ + sizeLength, nameLength);
                    DefinitionStyle* style = StyleParse(inText_ + start_ + sizeLength + nameLength, currentParamLength_ - sizeLength - nameLength);
                    model_->styles_[styleCount_] = style;
                    styleCount_++;
                }            
            }
            else
            {
                currentElement_->setStyle(StyleParse(inText_ + start_, currentParamLength_), DefinitionElement::ownStyle);
            }    
            break;

        case paramStyleName:
            {
                const DefinitionStyle* style = StyleGetStaticStyle(inText_ + start_, currentParamLength_);
                const DefinitionStyle* serverStyle = NULL;
                for (ulong_t i = 0; i < totalStyleCount_; i++)
                {
                    //TODO: this is ugly (but not broken - styles are sorted!)
                    if (StrEquals(styleNames_[i], inText_ + start_, currentParamLength_))
                    {
                        serverStyle = model_->styles_[i];
                        i = totalStyleCount_;
                    }
                }
                assert(serverStyle != NULL || style != NULL);
                if (serverStyle != NULL)
                    currentElement_->setStyle(serverStyle);
                else
                    currentElement_->setStyle(style);
            }
            break;
        
        case paramStylesCount:
            {
                //  free old styles
                for (ulong_t i = 0; i < totalStyleCount_; i++)
                    free(styleNames_[i]);
                delete [] styleNames_;
                styleNames_ = NULL;
                delete [] model_->styles_;
                model_->styles_ = NULL;
                model_->styleCount_ = 0;
                styleCount_ = 0;
                totalStyleCount_ = readUnaligned32(inText_ + start_);
                assert(totalStyleCount_ > 0);
                // alloc memory for new styles
                
                model_->styles_ = new_nt DefinitionStyle*[totalStyleCount_];
                if (NULL == model_->styles_)
                {
                    totalStyleCount_ = 0;
                    return false; // TODO: here it's broken by design - no way to return error code
                }    
                memzero(model_->styles_, sizeof(DefinitionStyle*) * totalStyleCount_);
                model_->styleCount_ = totalStyleCount_;
              
                styleNames_ = new_nt char*[totalStyleCount_];
                if (NULL == styleNames_)
                {
                    totalStyleCount_ = 0;
                    return false; // TODO: here it's broken by design - no way to return error code
                }    
                memzero(styleNames_, sizeof(char*) * totalStyleCount_);
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
            model_->elements.push_back(currentElement_ = new LineBreakElement());
            break;

        case typeHorizontalLineElement:
            model_->elements.push_back(currentElement_ = new HorizontalLineElement());
            break;

        case typeTextElement:
            model_->elements.push_back(currentElement_ = new TextElement());
            break;

        case typeBulletElement:
            model_->elements.push_back(currentElement_ = new BulletElement());
            break;

        case typeListNumberElement:
            model_->elements.push_back(currentElement_ = new ListNumberElement());
            break;

        case typeParagraphElement:
            model_->elements.push_back(currentElement_ = new ParagraphElement());
            break;

        case typeIndentedParagraphElement:
            model_->elements.push_back(currentElement_ = new ParagraphElement(true));
            break;

        case typeStylesTableElement:
            // clear styles table? paramStylesCount will do this
            break;
            
        case typeTitleElement:
            // this is first element
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
    if (NULL != currentElement_)
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
    currentElementType_ = readUnaligned32(inText_ + start_);
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
        ulong_t nextElementPos = start_ + currentElementParamsLength_;
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
    while(parseElement()) ;
    // remove parsed text from memory
	StrErase(inText_, inLength_, 0, start_); 
    inLength_ -= start_;
    totalSize_ -= start_;
    start_ = 0;
    return errNone;
}

bool ByteFormatParser::parseHeader(const char* inText)
{
    if (headerParsed_)
        return true;
    
    totalSize_ = readUnaligned32(inText + start_);
    start_ += sizeLength;
    totalElementsCount_ = readUnaligned32(inText + start_);
    elementsCount_ = 0;
    start_ += sizeLength;
    version_ = readUnaligned32(inText + start_);
    start_ += sizeLength;
    headerParsed_ = true;
    
    return true;
}
       
status_t ByteFormatParser::handleIncrement(const char* inputText, ulong_t& inputLength, bool finish)
{	
	inText_ = StrAppend(inText_, inLength_, inputText, inputLength);
	if (NULL == inText_)
		return memErrNotEnoughSpace;
		
    inLength_ += inputLength;
    start_ = 0;
    finish_ = finish;

    if (NULL == model_)
    {
        model_ = new_nt DefinitionModel();
        if (NULL == model_) 
            return memErrNotEnoughSpace;
    }           
    
    if (!headerParsed_)
    {
        if (inLength_ < headerLength)
            return errNone;
        parseHeader(inText_);    
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
        //assert(inLength_ == totalSize_);
    }
    return error;
}

status_t ByteFormatParser::parseAll(const char* inputText, long inputTextLen)
{
    start_ = 0;

    if (NULL == model_)
    {
        model_ = new_nt DefinitionModel();
        if (NULL == model_) 
            return memErrNotEnoughSpace;
    }           

    parseHeader(inputText);
    assert(totalSize_ == ulong_t(inputTextLen) || -1 == inputTextLen);
	
	free(inText_);
	inText_ = StringCopyN(inputText, totalSize_);
	if (NULL == inText_)
		return memErrNotEnoughSpace;
		 
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
