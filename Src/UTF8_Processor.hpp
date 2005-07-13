#ifndef ARSLEXIS_UTF8_PROCESSOR_HPP__
#define ARSLEXIS_UTF8_PROCESSOR_HPP__

#include <IncrementalProcessor.hpp>

status_t UTF8_ToNative(const char*& utfText, ulong_t& utfLen, char_t*& nativeText, ulong_t& nativeLen);

class UTF8_Processor: public BinaryIncrementalProcessor
{
	TextIncrementalProcessor* textProcessor_;
	bool textProcessorOwner_;
	char* input_;
	ulong_t inputLen_;
	char_t* output_;
	ulong_t outputLen_;

public:

	UTF8_Processor(TextIncrementalProcessor* chainedTextProcessor, bool textProcessorOwner = true);

    ~UTF8_Processor();

    status_t handleIncrement(const char* payload, ulong_t& length, bool finish);
    
    TextIncrementalProcessor* chainedTextProcessor() {return textProcessor_;}
    TextIncrementalProcessor* releaseChainedProcessor();

};

#ifndef NDEBUG
void test_UTF8_ToNative();
#endif

#endif // ARSLEXIS_UTF8_PROCESSOR_HPP__