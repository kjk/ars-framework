#include <UniversalDataFormat.hpp>
#include <set>
#include <map>
#include <Utility.hpp>

#ifdef __MWERKS__
# pragma far_code
#endif


UniversalDataFormat::UniversalDataFormat() {}

UniversalDataFormat::~UniversalDataFormat() {}

int UniversalDataFormat::getItemsCount()
{
    assert(headerSize == header.size());

    return headerSize;
}
   
int UniversalDataFormat::getItemElementsCount(int itemNo)
{
    assert(0 < itemNo && itemNo < header.size());

    return header[itemNo].size();
}
   
const ArsLexis::char_t* UniversalDataFormat::getItemText(int itemNo, int elemNo)
{
    assert(0 < itemNo && itemNo < header.size());
    assert(0 < elemNo && elemNo < header[itemNo].size());

    //calculate real offset of itemText by itemNo, elemNo
    //TODO: decide what informations will be stored in header 
    uint_t offset = header[itemNo][elemNo];
    return (ArsLexis::char_t*) &(data[offset]);
}
    
ArsLexis::String UniversalDataFormat::getItemTextAsString(int itemNo, int elemNo)
{
    return (ArsLexis::String) getItemText(itemNo, elemNo);
}
