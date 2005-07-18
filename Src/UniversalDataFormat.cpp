#include <UniversalDataFormat.hpp>
#include <set>
#include <map>
#include <Utility.hpp>
#include <Text.hpp>
#include <UTF8_Processor.hpp>

#ifdef __MWERKS__
# pragma far_code
#endif

UniversalDataFormat::UniversalDataFormat(): 
	headerSize_(0), 
	fNormalized_(false), 
	data_(NULL), 
	dataLen_(0),
	text_(NULL),
	textLen_(NULL)
{}

UniversalDataFormat::~UniversalDataFormat() 
{
	free(data_);
	free(text_);
}

/**
 *  Replaces elements lengths by offsets of items
 *  Inserts '\0' at the end of items
 */
void UniversalDataFormat::normalize() const 
{
    // run it only one time!
    if (fNormalized_)
        return;
    fNormalized_ = true;
    VectorRange offset = 0;
    for (uint_t i = 0; i < header_.size(); i++)
    {
        for (uint_t j = 0; j < header_[i].size(); j++)
        {
            VectorRange len = header_[i][j];
            header_[i][j] = offset;
            offset += len;
            data_[offset] = '\0';
            offset++;
        }
    }    
}

ulong_t UniversalDataFormat::getItemsCount() const
{
    assert(headerSize_ == header_.size());
    return headerSize_;
}
   
ulong_t UniversalDataFormat::getItemElementsCount(ulong_t itemNo) const
{
    assert(itemNo < header_.size());
    return header_[itemNo].size();
}
   
const char* UniversalDataFormat::getItemData(ulong_t itemNo, ulong_t elemNo) const
{
    assert(itemNo < header_.size());
    assert(elemNo < header_[itemNo].size());

    if (!fNormalized_)
        normalize();
        
    VectorRange offset = header_[itemNo][elemNo];
    return data_ + offset;
}

const char* UniversalDataFormat::getItemData(ulong_t itemNo, ulong_t elemNo, ulong_t& lenOut) const
{
    assert(itemNo < header_.size());
    assert(elemNo < header_[itemNo].size());

    if (!fNormalized_)
        normalize();
        
    VectorRange offset = header_[itemNo][elemNo];
    VectorRange nextOffset = 0;
    if (header_[itemNo].size() > elemNo+1)
        nextOffset = header_[itemNo][elemNo+1] - 1;
    else if (header_.size() > itemNo+1)
        nextOffset = header_[itemNo+1][0] - 1;
    else
        nextOffset = dataLen_ - 1;
        
    const char* txt = data_ + offset;
    lenOut = nextOffset - offset;
    return txt;
}

const char_t* UniversalDataFormat::getItemText(ulong_t itemNo, ulong_t elemNo) const
{
	ulong_t dataLen;
	const char* data = getItemData(itemNo, elemNo, dataLen);
	free(text_);
	text_ = UTF8_ToNative(data, dataLen, &textLen_);
	return text_;
}

const char_t* UniversalDataFormat::getItemText(ulong_t itemNo, ulong_t elemNo, ulong_t& lenOut) const
{
	ulong_t dataLen;
	const char* data = getItemData(itemNo, elemNo, dataLen);
	free(text_);
	text_ = UTF8_ToNative(data, dataLen, &textLen_);
	lenOut = textLen_;
	return text_;
}

long UniversalDataFormat::getItemNumericValue(ulong_t itemNo, ulong_t elemNo) const
{
	ulong_t len;
	const char* text = getItemData(itemNo, elemNo, len);

    long result;
    status_t error = numericValue(text, text + len, result);
    if (errNone != error)
        return 0;
        
    return result;
}

void UniversalDataFormat::swap(UniversalDataFormat& udf)
{
    std::swap(headerSize_, udf.headerSize_);
    std::swap(fNormalized_, udf.fNormalized_);
    header_.swap(udf.header_);
    std::swap(data_, udf.data_); 
	std::swap(text_, udf.text_);
	std::swap(textLen_, udf.textLen_); 
}

void UniversalDataFormat::reset()
{
    headerSize_ = 0;
    fNormalized_ = false;
    header_.clear();
	free(data_);
	data_ = NULL;
	dataLen_ = 0;
	free(text_);
	text_ = NULL;
	textLen_ = 0;
}
