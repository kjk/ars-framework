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
  
    long getItemTextAsPositiveLong(int itemNo, int elemNo) const;
    
    friend ArsLexis::status_t parseUniversalDataFormatTextLine(const ArsLexis::String& line, UniversalDataFormat& out, int& lineNo);
};

#endif