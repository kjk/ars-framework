#include <UniversalDataHandler.hpp>
#include <Text.hpp>
#include <Reader.hpp>
#include <DataStore.hpp>

#if defined(__MWERKS__)
# pragma far_code
#endif
    
using ArsLexis::status_t;

UniversalDataHandler::UniversalDataHandler():lineNo(0) {}

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
        else if (lineNo <= out.headerSize)
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
            out.header.push_back(vec);
        }
        else
        {
            if (lineNo == out.headerSize + 1)
                out.data.assign(line);
            else
                out.data.append(line);
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
    return parseUniversalDataFormatTextLine(line, universalData_,lineNo);
}

UniversalDataHandler::~UniversalDataHandler() {}

status_t readUniversalDataFromStream(ArsLexis::Reader& reader, UniversalDataFormat& out)
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