#ifndef __ARSLEXIS_UNIVERSAL_DATA_FORMAT_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_FORMAT_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <vector>

struct UniversalDataFormat
{
    typedef unsigned int VectorRange;
    typedef std::vector<VectorRange> Vector_t;
    typedef std::vector<Vector_t> VectorOfVectors_t;

private:

    int                 headerSize_;
    bool                fNormalized_;
    VectorOfVectors_t   header_;
    ArsLexis::String    data_;

    void normalize();

    void setHeaderSize(int size)
    {
        headerSize_ = size;
        fNormalized_ = false;    
    }
  
public:

    UniversalDataFormat();
    
    ~UniversalDataFormat();
    
    int getItemsCount() const ;
    
    int getItemElementsCount(int itemNo) const;
    
    const ArsLexis::char_t *getItemText(int itemNo, int elemNo) const;
    
    ArsLexis::String getItemTextAsString(int intemNo, int elemNo) const;
  
    /**
     * @return -1 if element doesn't represent valid positive number.
     * @warning It's broken. It may return also -1 because the number is really -1 but nobody cares about it. 
     * It may return a whole range of negative numbers, although the name states result is positive.
     */
    long getItemTextAsPositiveLong(int itemNo, int elemNo) const;
    
    friend ArsLexis::status_t parseUniversalDataFormatTextLine(const ArsLexis::String& line, UniversalDataFormat& out, int& lineNo);
};

#endif