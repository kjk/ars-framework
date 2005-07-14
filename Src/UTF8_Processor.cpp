#include <UTF8_Processor.hpp>
#include <Text.hpp>
#include <Utility.hpp>

// Code in namespace Unicode is published by the unicode.org
namespace Unicode {

typedef unsigned long	UTF32;	/* at least 32 bits */
typedef unsigned short	UTF16;	/* at least 16 bits */
typedef unsigned char	UTF8;	/* typically 8 bits */
typedef bool	Boolean; /* 0 or 1 */

/* Some fundamental constants */
static const UTF32 UNI_REPLACEMENT_CHAR = (UTF32)0x0000FFFD;
static const UTF32 UNI_MAX_BMP = (UTF32)0x0000FFFF;
static const UTF32 UNI_MAX_UTF16 = (UTF32)0x0010FFFF;

typedef enum {
	conversionOK, 		/* conversion successful */
	sourceExhausted,	/* partial character in source, but hit end */
	targetExhausted,	/* insuff. room in target for conversion */
	sourceIllegal		/* source sequence is illegal/malformed */
} ConversionResult;

typedef enum {
	strictConversion = 0,
	lenientConversion
} ConversionFlags;

ConversionResult ConvertUTF8toUTF16 (const UTF8** sourceStart, const UTF8* sourceEnd, 	UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF16toUTF8 (const UTF16** sourceStart, const UTF16* sourceEnd, UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);
		
Boolean isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd);

static const int halfShift  = 10; /* used for shifting by 10 bits */

static const UTF32 halfBase = 0x0010000UL;
static const UTF32 halfMask = 0x3FFUL;

static const UTF32 UNI_SUR_HIGH_START = (UTF32)0xD800;
static const UTF32 UNI_SUR_HIGH_END = (UTF32)0xDBFF;
static const UTF32 UNI_SUR_LOW_START = (UTF32)0xDC00;
static const UTF32 UNI_SUR_LOW_END = (UTF32)0xDFFF;

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
static const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
		     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF16toUTF8 (
	const UTF16** sourceStart, const UTF16* sourceEnd, 
	UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    const UTF16* source = *sourceStart;
    UTF8* target = *targetStart;
    while (source < sourceEnd) {
	UTF32 ch;
	unsigned short bytesToWrite = 0;
	const UTF32 byteMask = 0xBF;
	const UTF32 byteMark = 0x80; 
	const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
	ch = *source++;
	/* If we have a surrogate pair, convert to UTF32 first. */
	if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
	    /* If the 16 bits following the high surrogate are in the source buffer... */
	    if (source < sourceEnd) {
		UTF32 ch2 = *source;
		/* If it's a low surrogate, convert to UTF32. */
		if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
		    ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
			+ (ch2 - UNI_SUR_LOW_START) + halfBase;
		    ++source;
		} else if (flags == strictConversion) { /* it's an unpaired high surrogate */
		    --source; /* return to the illegal value itself */
		    result = sourceIllegal;
		    break;
		}
	    } else { /* We don't have the 16 bits following the high surrogate. */
		--source; /* return to the high surrogate */
		result = sourceExhausted;
		break;
	    }
	} else if (flags == strictConversion) {
	    /* UTF-16 surrogate values are illegal in UTF-32 */
	    if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
		--source; /* return to the illegal value itself */
		result = sourceIllegal;
		break;
	    }
	}
	/* Figure out how many bytes the result will require */
	if (ch < (UTF32)0x80) {	     bytesToWrite = 1;
	} else if (ch < (UTF32)0x800) {     bytesToWrite = 2;
	} else if (ch < (UTF32)0x10000) {   bytesToWrite = 3;
	} else if (ch < (UTF32)0x110000) {  bytesToWrite = 4;
	} else {			    bytesToWrite = 3;
					    ch = UNI_REPLACEMENT_CHAR;
	}

	target += bytesToWrite;
	if (target > targetEnd) {
	    source = oldSource; /* Back up source pointer! */
	    target -= bytesToWrite; result = targetExhausted; break;
	}
	switch (bytesToWrite) { /* note: everything falls through. */
	    case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
	    case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
	    case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
	    case 1: *--target =  (UTF8)(ch | firstByteMark[bytesToWrite]);
	}
	target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    return result;
}

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *  length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

static Boolean isLegalUTF8(const UTF8 *source, int length) {
    UTF8 a;
    const UTF8 *srcptr = source+length;
    switch (length) {
    default: return false;
	/* Everything else falls through when "true"... */
    case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
    case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
    case 2: if ((a = (*--srcptr)) > 0xBF) return false;

	switch (*source) {
	    /* no fall-through in this inner switch */
	    case 0xE0: if (a < 0xA0) return false; break;
	    case 0xED: if (a > 0x9F) return false; break;
	    case 0xF0: if (a < 0x90) return false; break;
	    case 0xF4: if (a > 0x8F) return false; break;
	    default:   if (a < 0x80) return false;
	}

    case 1: if (*source >= 0x80 && *source < 0xC2) return false;
    }
    if (*source > 0xF4) return false;
    return true;
}

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd) {
    int length = trailingBytesForUTF8[*source]+1;
    if (source+length > sourceEnd) {
	return false;
    }
    return isLegalUTF8(source, length);
}

ConversionResult ConvertUTF8toUTF16 (
	const UTF8** sourceStart, const UTF8* sourceEnd, 
	UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    const UTF8* source = *sourceStart;
    UTF16* target = *targetStart;
    while (source < sourceEnd) {
	UTF32 ch = 0;
	unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
	if (source + extraBytesToRead >= sourceEnd) {
	    result = sourceExhausted; break;
	}
	/* Do this check whether lenient or strict */
	if (! isLegalUTF8(source, extraBytesToRead+1)) {
	    result = sourceIllegal;
	    break;
	}
	/*
	 * The cases all fall through. See "Note A" below.
	 */
	switch (extraBytesToRead) {
	    case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
	    case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
	    case 3: ch += *source++; ch <<= 6;
	    case 2: ch += *source++; ch <<= 6;
	    case 1: ch += *source++; ch <<= 6;
	    case 0: ch += *source++;
	}
	ch -= offsetsFromUTF8[extraBytesToRead];

	if (target >= targetEnd) {
	    source -= (extraBytesToRead+1); /* Back up source pointer! */
	    result = targetExhausted; break;
	}
	if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
	    /* UTF-16 surrogate values are illegal in UTF-32 */
	    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
		if (flags == strictConversion) {
		    source -= (extraBytesToRead+1); /* return to the illegal value itself */
		    result = sourceIllegal;
		    break;
		} else {
		    *target++ = UNI_REPLACEMENT_CHAR;
		}
	    } else {
		*target++ = (UTF16)ch; /* normal case */
	    }
	} else if (ch > UNI_MAX_UTF16) {
	    if (flags == strictConversion) {
		result = sourceIllegal;
		source -= (extraBytesToRead+1); /* return to the start */
		break; /* Bail out; shouldn't continue */
	    } else {
		*target++ = UNI_REPLACEMENT_CHAR;
	    }
	} else {
	    /* target is a character in range 0xFFFF - 0x10FFFF. */
	    if (target + 1 >= targetEnd) {
		source -= (extraBytesToRead+1); /* Back up source pointer! */
		result = targetExhausted; break;
	    }
	    ch -= halfBase;
	    *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
	    *target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
	}
    }
    *sourceStart = source;
    *targetStart = target;
    return result;
}

}

#ifdef _WIN32_WCE

static const StaticAssert<sizeof(char_t) == sizeof(Unicode::UTF16)> assert_char_t_is_utf16;
static const StaticAssert<long(char_t(-1)) == long(Unicode::UTF16(-1))> assert_char_t_has_same_singularity_as_utf16;

status_t UTF8_ToNative(const char*& utfText, ulong_t& utfLen, char_t*& nativeText, ulong_t& nativeLen)
{
	using namespace Unicode;
	nativeText = NULL;
	nativeLen = 0;
	if (0 == utfLen)
		return errNone;

	ulong_t bufLen = utfLen;
	nativeText = (char_t*)malloc(bufLen * sizeof(char_t) + 1);
	if (NULL == nativeText)
		return memErrNotEnoughSpace;

	status_t err = errNone;
	char_t* target = nativeText;
	const char* source = utfText;
	ConversionResult res = ConvertUTF8toUTF16((const UTF8**)&utfText, (const UTF8*)(source + utfLen), (UTF16**)&target, (UTF16*)(target + bufLen), lenientConversion);
	if (sourceIllegal == res)
		err = sysErrParamErr;
	else
	{
		utfLen -= (utfText - source);
		nativeLen = (target - nativeText);
		if (0 != nativeLen)
			*target = L'\0';
	}
	if (errNone != err && NULL != nativeText)
	{
		free(nativeText);
		nativeText = NULL;
	}
	return err;
}

status_t UTF8_FromNative(const char_t*& nativeText, ulong_t& nativeLen, char*& utfText, ulong_t& utfLen)
{
	using namespace Unicode;
	utfText = NULL;
	utfLen = 0;
	if (0 == nativeLen)
		return errNone;

	ulong_t bufLen = nativeLen;
	utfText = (char*)malloc(bufLen * sizeof(char) + 1);
	if (NULL == utfText)
		return memErrNotEnoughSpace;

	status_t err = errNone;
	char* target = utfText;
	const char_t* source = nativeText;
	ConversionResult res = ConvertUTF16toUTF8((const UTF16**)&nativeText, (const UTF16*)(source + nativeLen), (UTF8**)&target, (UTF8*)(target + bufLen), lenientConversion);
	if (sourceIllegal == res)
		err = sysErrParamErr;
	else
	{
		nativeLen -= (nativeText - source);
		utfLen = (target - utfText);
		if (0 != utfLen)
			*target = '\0';
	}
	if (errNone != err && NULL != utfText)
	{
		free(utfText);
		utfText = NULL;
	}
	return err;
}

#elif defined(_PALM_OS)

status_t UTF8_ToNative(const char*& utfText, ulong_t& utfLen, char*& nativeText, ulong_t& nativeLen)
{
	using namespace Unicode;
	nativeText = NULL;
	nativeLen = 0;
	if (0 == utfLen)
		return errNone;

	ulong_t bufLen = utfLen;
	UTF16* buffer = (UTF16*)malloc(bufLen * sizeof(UTF16));
	if (NULL == buffer)
		return memErrNotEnoughSpace;

	status_t err = errNone;
	UTF16* target = buffer;
	const char* source = utfText;
	ConversionResult res = ConvertUTF8toUTF16((const UTF8**)&utfText, (const UTF8*)(source + utfLen), &target, target + bufLen, lenientConversion);
	if (sourceIllegal == res)
	{
		free(buffer);
		return sysErrParamErr;
	}
	utfLen -= (utfText - source);
	bufLen = (target - buffer);
	nativeText = (char*)malloc(bufLen + 1);
	if (NULL == nativeText)
	{
		free(buffer);
		return memErrNotEnoughSpace;
	}
	for (ulong_t i = 0; i < bufLen; ++i)
	{
		if (buffer[i] < 256)
			nativeText[i] = char((unsigned char)buffer[i]);
		else
			nativeText[i] = char(1);
	}
	free(buffer);
	nativeText[bufLen] = '\0';
	nativeLen = bufLen;
	return errNone;
}

status_t UTF8_FromNative(const char_t*& nativeText, ulong_t& nativeLen, char*& utfText, ulong_t& utfLen)
{
	using namespace Unicode;
	utfText = NULL;
	utfLen = 0;
	if (0 == nativeLen)
		return errNone;

    UTF16* buffer = (UTF16*)malloc(sizeof(UTF16) * nativeLen);
    if (NULL == buffer)
        return memErrNotEnoughSpace;
        
    ulong_t targetLen = 0;
    for (ulong_t i = 0; i < nativeLen; ++i)
    {
        ++targetLen;
        UTF16 c = UTF16((unsigned char)nativeText[i]);
        if (c > 0x7f)
            ++targetLen;
        
        buffer[i] = c;
    }
        
    utfText = (char*)malloc(targetLen + 1);
    if (NULL == utfText)
    {
        free(buffer);
        return memErrNotEnoughSpace;
    }
    
    const UTF16* source = buffer;
    char* target = utfText;
    
    ConversionResult res = ConvertUTF16toUTF8(&source, source + nativeLen, (UTF8**)&target, (UTF8*)(target + targetLen), lenientConversion);
    if (sourceIllegal != res)
    {
        nativeText += (source - buffer);
        nativeLen -= (source - buffer);
        assert(0 == nativeLen);
        utfLen = (target - utfText);
        assert(targetLen == utfLen);
        *target = '\0';
    }
    else 
    {
        free(utfText);
        utfText = NULL;
    }
    free(buffer);
    if (sourceIllegal == res)
        return sysErrParamErr;
    return errNone;
}


#else

#error "define UTF8_ToNative() for yer platform!"

#endif // _WIN32_WCE

#ifndef NDEBUG

#ifdef _WIN32_WCE
#pragma setlocale("plk")

void test_UTF8_ToNative()
{
	const char* test = "test test test";
	ulong_t len = strlen(test);
	tchar* nstr;
	ulong_t nlen;
	status_t res = UTF8_ToNative(test, len, nstr, nlen);
	assert(errNone == res);
	assert(NULL != nstr);
	assert(0 == len);
	assert(0 == wcsncmp(L"test test test", nstr, nlen));
	free(nstr);

	test = "Zażółć gęślą jaźń!";
	len = strlen(test);
	res = UTF8_ToNative(test, len, nstr, nlen);
	assert(errNone == res);
	assert(NULL != nstr);
	assert(0 == len);
	assert(0 == wcsncmp(L"Za��� g�l� ja��!", nstr, nlen));
	free(nstr);
}
#else
void test_UTF8_ToNative()
{
	const char* test = "test test test";
	ulong_t len = strlen(test);
	char* nstr;
	ulong_t nlen;
	status_t res = UTF8_ToNative(test, len, nstr, nlen);
	assert(errNone == res);
	assert(NULL != nstr);
	assert(0 == len);
	assert(equals(test, nstr));
	free(nstr);

	test = "Zażółć gęślą jaźń!";
	len = strlen(test);
	res = UTF8_ToNative(test, len, nstr, nlen);
	assert(errNone == res);
	assert(NULL != nstr);
	assert(0 == len);
	free(nstr);
}

void test_UTF8_FromNative()
{
    const char* test = "test�j";
    ulong_t len = strlen(test);
    ulong_t olen;
    char* out = UTF8_FromNative(test, len, &olen);
    assert(NULL != out);
    assert(olen == len + 1);
    free(out);
}

#endif

#endif

char_t* UTF8_ToNative(const char* utfText, long len, ulong_t* olen)
{
	if (-1 == len) len = Len(utfText);
	char_t* out;
	ulong_t outLen;
	ulong_t inLen = len;
	status_t err = UTF8_ToNative(utfText, inLen, out, outLen);
	if (errNone != err)
		return NULL;
	
	if (NULL != olen)
		*olen = outLen;
	
	return out;
}

char* UTF8_FromNative(const char_t* nativeText, long len, ulong_t* olen)
{
	if (-1 == len) len = Len(nativeText);
	char* out;
	ulong_t outLen;
	ulong_t inLen = len;
	status_t err = UTF8_FromNative(nativeText, inLen, out, outLen);
	if (errNone != err)
		return NULL;
	
	if (NULL != olen)
		*olen = outLen;
	
	return out;
}


UTF8_Processor::UTF8_Processor(TextIncrementalProcessor* chainedTextProcessor, bool textProcessorOwner):
	textProcessor_(chainedTextProcessor),
	textProcessorOwner_(textProcessorOwner),
	input_(NULL),
	inputLen_(0),
	output_(NULL),
	outputLen_(0)
{}

UTF8_Processor::~UTF8_Processor()
{
	if (textProcessorOwner_)
		delete textProcessor_;
	free(input_);
	free(output_);
}

status_t UTF8_Processor::handleIncrement(const char* payload, ulong_t& length, bool finish)
{
	input_ = StrAppend(input_, inputLen_, payload, length);
	if (NULL == input_)
		return memErrNotEnoughSpace;
        
    inputLen_ += length;
	const char* input = input_;
	ulong_t inputLen = inputLen_;
	char_t* output = NULL;
	ulong_t outputLen = 0;

	status_t err = UTF8_ToNative(input, inputLen, output, outputLen);
	if (errNone != err)
		return err;

	StrErase(input_, inputLen_, 0, inputLen_ - inputLen);
	inputLen_ = inputLen;

	output_ = StrAppend(output_, outputLen_, output, outputLen);
	free(output);
	if (NULL == output_)
	{
		outputLen_ = 0;
		return memErrNotEnoughSpace;
	}
	outputLen_ += outputLen;
	if (NULL == textProcessor_)
		return errNone;

	outputLen = outputLen_;
	err = textProcessor_->handleIncrement(output_, outputLen, finish);
	if (errNone != err)
		return err;

	StrErase(output_, outputLen_, 0, outputLen);
	outputLen_ -= outputLen;
	return errNone;
}

TextIncrementalProcessor* UTF8_Processor::releaseChainedProcessor()
{
    TextIncrementalProcessor* p = textProcessor_;
    textProcessor_ = NULL;
    textProcessorOwner_ = false;
    return p;
}
