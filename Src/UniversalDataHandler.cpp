#include <UniversalDataHandler.hpp>
#include <Text.hpp>
#include <Reader.hpp>
#include <DataStore.hpp>

#if defined(__MWERKS__)
# pragma far_code
#endif

using namespace ArsLexis;

static status_t openDataStoreReader(const char_t* name, DataStoreReaderPtr& reader)
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

static status_t openDataStoreWriter(const ArsLexis::char_t* name, DataStoreWriterPtr& writer)
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



UniversalDataHandler::UniversalDataHandler():lineNo(0), writerStreamName_(NULL), fIsWriterAvailable_(false), writer_(NULL) {}

UniversalDataHandler::UniversalDataHandler(const ArsLexis::char_t* streamName):
    lineNo(0),
    writerStreamName_(streamName),
    fIsWriterAvailable_(true)
{
    openDataStoreWriter(writerStreamName_, writer_);
}

status_t parseUniversalDataFormatTextLine(const ArsLexis::String& line, UniversalDataFormat& out, int& lineNo)
{
    volatile status_t error=errNone;
    using namespace std;
    using namespace ArsLexis;        
    long resultLong;
    const char_t* data = line.data();
    const String::size_type len = line.length();
    ErrTry {
        if (lineNo == 0)
        {
            if(errNone != numericValue(data, data + len, resultLong))
                return SocketConnection::errResponseMalformed;
            out.setHeaderSize(resultLong);
        }
        else if (lineNo <= out.headerSize_)
        {
            UniversalDataFormat::Vector_t vec;
            //read values from vector
            const char_t* dataOffset = data;
            while (data + len > dataOffset)
            {
                const char_t* dataOffsetEnd = dataOffset;
                while (dataOffsetEnd < data + len && dataOffsetEnd[0] != _T(' '))
                    dataOffsetEnd++;
                if(errNone != numericValue(dataOffset, dataOffsetEnd, resultLong))
                    return SocketConnection::errResponseMalformed;
                vec.push_back(resultLong);
                dataOffset = dataOffsetEnd + 1;
            }
            out.header_.push_back(vec);
        }
        else
        {
            if (lineNo == out.headerSize_ + 1)
                out.data_.assign(line);
            else
                out.data_.append(line);
        }
         lineNo++;
    }
    ErrCatch (ex) {
        error=ex;
    } ErrEndCatch
    return error;
}

status_t UniversalDataHandler::handleLine(const ArsLexis::String& line)
{
    if (fIsWriterAvailable_)
        writeLineToDataStore(writer_, line);
    return parseUniversalDataFormatTextLine(line, universalData_,lineNo);
}

UniversalDataHandler::~UniversalDataHandler() {}

static status_t readUniversalDataFromReader(ArsLexis::Reader& reader, UniversalDataFormat& out)
{
    int lineNo = 0;
    bool eof = false;
    while (!eof)
    {
        ArsLexis::String line;
        ArsLexis::status_t error=reader.readLine(eof, line);
        if (errNone!=error)
            return error;
        if (errNone!=(error=parseUniversalDataFormatTextLine(line, out, lineNo)))
            return error;
    }
    return errNone;
}

void readUniversalDataFromStream(const char_t* streamName, UniversalDataFormat& out)
{
    DataStoreReaderPtr reader;
    status_t error = openDataStoreReader(streamName, reader);
    if (errNone != error)
        return;
    readUniversalDataFromReader(*reader, out);
}
