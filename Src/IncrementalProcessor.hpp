#ifndef ARSLEXIS_INCREMENTAL_PROCESSOR_HPP__
#define ARSLEXIS_INCREMENTAL_PROCESSOR_HPP__

#include <Utility.hpp>

class BinaryIncrementalProcessor: private NonCopyable
{
public:

    virtual ~BinaryIncrementalProcessor();

    virtual status_t handleIncrement(const char* payload, ulong_t& length, bool finish) = 0;
};

class TextIncrementalProcessor: private NonCopyable
{
public:

    virtual ~TextIncrementalProcessor();

    virtual status_t handleIncrement(const char_t* payload, ulong_t& length, bool finish) = 0;
};

class Reader;

status_t FeedHandlerFromReader(BinaryIncrementalProcessor& handler, Reader& reader);

#endif // ARSLEXIS_INCREMENTAL_PROCESSOR_HPP__