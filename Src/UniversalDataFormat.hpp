#ifndef __ARSLEXIS_UNIVERSAL_DATA_FORMAT_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_FORMAT_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <vector>
#include <Utility.hpp>

struct UniversalDataFormat //: private ArsLexis::NonCopyable
{
    typedef ulong_t VectorRange;
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

    const ArsLexis::char_t *getItemTextAndLen(int itemNo, int elemNo, ulong_t* lenOut) const;
    
    ArsLexis::String getItemTextAsString(int intemNo, int elemNo) const;
  
    /**
     * without error check
     * @return 0 if element doesn't represent valid number.
     * @return 0 if element is equal 0 too
     */
    long getItemTextAsLong(int itemNo, int elemNo) const;
    
    void swap(UniversalDataFormat& udf);
    
    void reset();
    
    friend ArsLexis::status_t parseUniversalDataFormatTextLine(const ArsLexis::String& line, UniversalDataFormat& out, int& lineNo, ulong_t& controlDataLength);

    friend class UniversalDataHandler;
    
    bool empty() const {return header_.empty();}
    
    ulong_t dataLength() {return data_.length();}
    
};

template<> 
inline void std::swap(UniversalDataFormat& udf0, UniversalDataFormat& udf1)
{udf0.swap(udf1);}

#endif
