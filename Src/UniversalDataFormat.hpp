#ifndef __ARSLEXIS_UNIVERSAL_DATA_FORMAT_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_FORMAT_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <vector>
#include <Utility.hpp>

struct UniversalDataFormat: private NonCopyable
{
    typedef ulong_t VectorRange;
    typedef std::vector<VectorRange> Vector_t;
    typedef std::vector<Vector_t> VectorOfVectors_t;

    ulong_t                 headerSize_;
    mutable bool         fNormalized_;
    mutable VectorOfVectors_t   header_;
    mutable char*		data_;
	ulong_t		dataLen_; 

    void normalize() const;

    void setHeaderSize(int size)
    {
        headerSize_ = size;
        fNormalized_ = false;    
    }
   
   mutable char_t* text_;
   mutable ulong_t textLen_; 
  
public:

    UniversalDataFormat();
    
    ~UniversalDataFormat();
    
    ulong_t getItemsCount() const ;
    
    ulong_t getItemElementsCount(ulong_t itemNo) const;
    
    const char* getItemData(ulong_t itemNo, ulong_t elemNo) const;

    const char* getItemData(ulong_t itemNo, ulong_t elemNo, ulong_t& lenOut) const;
   
	const char_t* getItemText(ulong_t itemNo, ulong_t elemNo) const;
	const char_t* getItemText(ulong_t itemNo, ulong_t elemNo, ulong_t& lenOut) const;
    
     
    /**
     * without error check
     * @return 0 if element doesn't represent valid number.
     * @return 0 if element is equal 0 too
     */
    long getItemNumericValue(ulong_t itemNo, ulong_t elemNo) const;
    
    void swap(UniversalDataFormat& udf);
    
    void reset();
    
    friend status_t parseUniversalDataFormatTextLine(const NarrowString& line, UniversalDataFormat& out, int& lineNo, ulong_t& controlDataLength);

    friend class UniversalDataHandler;
    
    bool empty() const {return header_.empty();}
    
    ulong_t dataLength() {return dataLen_;}
    
};

/*
template<> 
inline void std::swap(UniversalDataFormat& udf0, UniversalDataFormat& udf1)
{udf0.swap(udf1);}
*/

#endif
