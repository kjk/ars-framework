#include <UniversalDataHandler.hpp>
#include <SocketConnection.hpp>
#include <Text.hpp>

#include <Reader.hpp>
#include <DataStore.hpp>
#include <BufferedReader.hpp>
 
#if defined(__MWERKS__)
# pragma far_code
#endif

static status_t openDataStoreReader(const char* name, DataStoreReaderPtr& reader)
{
    DataStore* ds=DataStore::instance();
    if (NULL == ds)
        return DataStore::errNotFound;
    DataStoreReaderPtr dsr(new DataStoreReader(*ds));
    status_t error = dsr->open(name);
    if (errNone != error)
        return error;
    reader = dsr;
    return errNone;
}

static status_t openDataStoreWriter(const char* name, DataStoreWriterPtr& writer)
{
    DataStore* ds = DataStore::instance();
    if (NULL == ds)
        return DataStore::errNotFound;
    DataStoreWriterPtr dsw(new DataStoreWriter(*ds));
    status_t error = dsw->open(name);
    if (errNone != error)
        return error;
    writer = dsw;
    return errNone;
}
 /*

static void writeLineToDataStore(DataStoreWriterPtr& writer, const String& line)
{
    if (NULL == writer.get())
        return;
    status_t error = writer->write(line.data(), line.length());
    if (errNone != error)
        goto OnError;
    error = writer->write(_T('\n'));
    if (errNone != error)
        goto OnError;
    return;
OnError:
    writer.reset();
    //! @todo remove stream from data store?
}
 */
 
UniversalDataHandler::UniversalDataHandler():
    lineNo_(0), 
    controlDataLength_(0)
{}

status_t parseUniversalDataFormatTextLine(const char* line, ulong_t len, UniversalDataFormat& out, ulong_t& lineNo, ulong_t& controlDataLength)
{
    long resultLong;
    const char* data = line; 
    UniversalDataFormat::Vector_t vec;
    ErrTry {
        if (lineNo == 0)
        {
            if (errNone != numericValue(data, data + len, resultLong))
                ErrReturn(SocketConnection::errResponseMalformed);

            out.setHeaderSize(resultLong);
        }
        else if (lineNo <= out.headerSize_)
        {
            //read values from vector
            const char* dataOffset = data;
            while (data + len > dataOffset)
            {
                const char* dataOffsetEnd = dataOffset;
                while (dataOffsetEnd < data + len && dataOffsetEnd[0] != ' ')
                    dataOffsetEnd++;
                    
                if (errNone != numericValue(dataOffset, dataOffsetEnd, resultLong))
		          ErrReturn(SocketConnection::errResponseMalformed);

                vec.push_back(resultLong);
                controlDataLength += resultLong + 1;
                dataOffset = dataOffsetEnd + 1;
            }
            out.header_.push_back(vec);
        }
        else
        {
            if (lineNo == out.headerSize_ + 1)
            {
                free(out.data_);
                out.dataLen_ = 0; 
                out.data_ = StringCopyN(line, len);
                if (NULL == out.data_)
					ErrReturn(memErrNotEnoughSpace);
						
				out.dataLen_ = len; 
            } 
            else
            {
				out.data_ = StrAppend(out.data_, out.dataLen_, "\n", 1);
				if (NULL == out.data_)
					ErrReturn(memErrNotEnoughSpace);
				
				++out.dataLen_;
				out.data_ = StrAppend(out.data_, out.dataLen_, line, len);
				if (NULL == out.data_)
					ErrReturn(memErrNotEnoughSpace);
					
				out.dataLen_ += len;
            }
        }
        lineNo++;
    }
    ErrCatch (ex) {
        return ex;
    } ErrEndCatch
    return errNone;
}

status_t UniversalDataHandler::handleLine(const char* line, ulong_t len)
{
//    writeLineToDataStore(writer_, line);
    return parseUniversalDataFormatTextLine(line, len, universalData, lineNo_, controlDataLength_);
}

status_t UniversalDataHandler::handlePayloadFinish()
{
    ulong_t realDataLen = universalData.dataLength();
    assert(controlDataLength_ == realDataLen);
    if (controlDataLength_ != realDataLen)
        return SocketConnection::errResponseMalformed;
    return errNone;
}

status_t UniversalDataHandler::handleIncrement(const char* payload, ulong_t& length, bool finish)
{
    status_t error = LineBufferedNarrowProcessor::handleIncrement(payload, length, finish);
    if (errNone == error && finish)
        return handlePayloadFinish();
    return error;
}

UniversalDataHandler::~UniversalDataHandler() {}

/*
status_t readUniversalDataFromReader(Reader& origReader, UniversalDataFormat& out)
{
    BufferedReader reader(origReader, 1024);
    int lineNo = 0;
    ulong_t controlDataLength = 0;
    bool eof = false;
    out.reset();
    while (!eof)
    {
        ArsLexis::String line;
        ArsLexis::status_t error=reader.readLine(eof, line);
        if (errNone!=error)
            return error;
        if (eof && line.empty()) //writer puts '\n' after last line... 
            break;
        error = parseUniversalDataFormatTextLine(line, out, lineNo, controlDataLength);
        if (errNone!=error)
            return error;
    }
    assert(controlDataLength == out.dataLength());
    if (controlDataLength != out.dataLength())
        return SocketConnection::errResponseMalformed;
    return errNone;
}

void readUniversalDataFromStream(const char_t* streamName, UniversalDataFormat& out)
{
    DataStoreReaderPtr reader;
    status_t error = openDataStoreReader(streamName, reader);
    if (errNone != error)
        return;
    if (errNone != readUniversalDataFromReader(*reader, out))
        out.reset(); //any errors - return empty UDF
}
 */
 