#include <UniversalDataFormat.hpp>
#include <set>
#include <map>
#include <Utility.hpp>

#ifdef __MWERKS__
# pragma far_code
#endif

UniversalDataFormat::UniversalDataFormat() {}

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

int UniversalDataFormat::getItemsCount()
{
    assert(headerSize_ == header_.size());

    return headerSize_;
}
   
int UniversalDataFormat::getItemElementsCount(int itemNo)
{
    assert(0 <= itemNo && itemNo < header_.size());

    return header_[itemNo].size();
}
   
const ArsLexis::char_t* UniversalDataFormat::getItemText(int itemNo, int elemNo)
{
    assert(0 <= itemNo && itemNo < header_.size());
    assert(0 <= elemNo && elemNo < header_[itemNo].size());

    if (!fNormalized_)
        normalize();
    //get offset of data
    uint_t offset = header_[itemNo][elemNo];
    return data_.data() + offset;
}
    
ArsLexis::String UniversalDataFormat::getItemTextAsString(int itemNo, int elemNo)
{
    return getItemText(itemNo, elemNo);
}
