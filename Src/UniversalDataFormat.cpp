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
    if (fNormalized)
        return;
    fNormalized = true;
    VectorRange offset = 0;
    for (uint_t i = 0; i < header.size(); i++)
    {
        for (uint_t j = 0; j < header[i].size(); j++)
        {
            VectorRange len = header[i][j];
            header[i][j] = offset;
            offset += len;
            data[offset] = _T('\0');
            offset++;
        }
    }    
}

int UniversalDataFormat::getItemsCount()
{
    assert(headerSize == header.size());

    return headerSize;
}
   
int UniversalDataFormat::getItemElementsCount(int itemNo)
{
    assert(0 <= itemNo && itemNo < header.size());

    return header[itemNo].size();
}
   
const ArsLexis::char_t* UniversalDataFormat::getItemText(int itemNo, int elemNo)
{
    assert(0 <= itemNo && itemNo < header.size());
    assert(0 <= elemNo && elemNo < header[itemNo].size());

    if (!fNormalized)
        normalize();
    //get offset of data
    uint_t offset = header[itemNo][elemNo];
    return (ArsLexis::char_t*) &(data[offset]);
}
    
ArsLexis::String UniversalDataFormat::getItemTextAsString(int itemNo, int elemNo)
{
    return (ArsLexis::String) getItemText(itemNo, elemNo);
}
