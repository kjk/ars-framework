#include <LineBufferedPayloadHandler.hpp>
#include <Text.hpp>

LineBufferedTextProcessor::LineBufferedTextProcessor():
	buffer_(NULL),
	bufferLen_(0),
    delimiter_(_T('\n'))
{}

LineBufferedTextProcessor::~LineBufferedTextProcessor()
{
	free(buffer_);
}

status_t LineBufferedTextProcessor::handleIncrement(const char_t* payload, ulong_t& length, bool finish)
{
	buffer_ = StrAppend(buffer_, bufferLen_, payload, length);
	if (NULL == buffer_)
		return memErrNotEnoughSpace;

	bufferLen_ += length;
	long pos = StrFind(buffer_, bufferLen_, delimiter_);
    while (true) 
    {
        if (-1 == pos && !finish)
            break;
		
		const char_t* line = buffer_;
        status_t error=handleLine(line, pos);
        if (errNone != error)
            return error;

        if (-1 == pos)
            break;

		StrErase(buffer_, bufferLen_, 0, pos + 1);
		bufferLen_ -= pos + 1;
        pos = StrFind(buffer_, bufferLen_, delimiter_);
    }
    return errNone;
}


LineBufferedNarrowProcessor::LineBufferedNarrowProcessor():
	buffer_(NULL),
	bufferLen_(0),
    delimiter_('\n')
{}

LineBufferedNarrowProcessor::~LineBufferedNarrowProcessor()
{
	free(buffer_);
}

status_t LineBufferedNarrowProcessor::handleIncrement(const char* payload, ulong_t& length, bool finish)
{
	buffer_ = StrAppend(buffer_, bufferLen_, payload, length);
	if (NULL == buffer_)
		return memErrNotEnoughSpace;

	bufferLen_ += length;
	long pos = StrFind(buffer_, bufferLen_, delimiter_);
    while (true) 
    {
        if (-1 == pos && !finish)
            break;
		
		const char* line = buffer_;
        status_t error=handleLine(line, pos);
        if (errNone != error)
            return error;

        if (-1 == pos)
            break;

		StrErase(buffer_, bufferLen_, 0, pos + 1);
		bufferLen_ -= pos + 1;
        pos = StrFind(buffer_, bufferLen_, delimiter_);
    }
    return errNone;
}
