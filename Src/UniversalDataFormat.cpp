#include <UniversalDataFormat.hpp>
#include <set>
#include <map>
#include <Utility.hpp>
#include <Text.hpp>

#ifdef __MWERKS__
# pragma far_code
#endif

UniversalDataFormat::UniversalDataFormat(): headerSize_(0), fNormalized_(false) {}

UniversalDataFormat::~UniversalDataFormat() {}

/**
 *  Replaces elements lengths by offsets of items
 *  Inserts '\0' at the end of items
 */
void UniversalDataFormat::normalize()
{
    //run it only one time!
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
            data_[offset] = _T('\0');
            offset++;
        }
    }    
}

int UniversalDataFormat::getItemsCount() const
{
    assert(headerSize_ == header_.size());

    return headerSize_;
}
   
int UniversalDataFormat::getItemElementsCount(int itemNo) const
{
    assert(0 <= itemNo && itemNo < header_.size());

    return header_[itemNo].size();
}
   
const ArsLexis::char_t* UniversalDataFormat::getItemText(int itemNo, int elemNo) const
{
    assert(0 <= itemNo && itemNo < header_.size());
    assert(0 <= elemNo && elemNo < header_[itemNo].size());

    if (!fNormalized_)
        const_cast<UniversalDataFormat*>(this)->normalize();
    //get offset of data
    uint_t offset = header_[itemNo][elemNo];
    return data_.data() + offset;
}
    
ArsLexis::String UniversalDataFormat::getItemTextAsString(int itemNo, int elemNo) const
{
    return getItemText(itemNo, elemNo);
}

// -1 means error
long UniversalDataFormat::getItemTextAsPositiveLong(int itemNo, int elemNo) const
{
    const ArsLexis::char_t* text = getItemText(itemNo, elemNo);
    long result;
    ArsLexis::status_t error = ArsLexis::numericValue(text, text+tstrlen(text), result);
    if (errNone != error)
        return -1;
    return result;
}

void UniversalDataFormat::swap(UniversalDataFormat& udf)
{
    std::swap(headerSize_, udf.headerSize_);
    std::swap(fNormalized_, udf.fNormalized_);
    header_.swap(udf.header_);
    data_.swap(udf.data_);
}

void UniversalDataFormat::reset()
{
    headerSize_ = 0;
    fNormalized_ = false;
    header_.clear();
    data_.assign(1,_T('\0'));
}