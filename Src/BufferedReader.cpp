#include <BufferedReader.hpp>

BufferedReader::BufferedReader(Reader& reader, uint_t chunkSize):
    reader_(reader),
    isMarked_(false),
    position_(0),
    chunkSize_(chunkSize)
{}

BufferedReader::~BufferedReader() {}

void BufferedReader::mark()
{
    assert(position_ <= buffer_.size());
    buffer_.erase(0, position_);
    position_ = 0;
    isMarked_ = true;
}

void BufferedReader::unmark()
{
    assert(isMarked_);
    buffer_.erase(0, position_);
    position_ = 0;
    isMarked_ = false;
}

status_t  BufferedReader::seek(SeekOffset offset, SeekType type) 
{
    assert(isMarked_);
    if (!isMarked_) 
        return sysErrNotAllowed;
    Buffer_t::size_type size = buffer_.size();
    switch (type)
    {
        case seekFromBeginning:
            if (size < offset)
                return sysErrParamErr;
            position_ = offset;
            break;
            
        case seekFromEnd:
            if (0 < offset || size < -offset)
                return sysErrParamErr;
            position_ = SeekOffset(size) + offset;
            break;
            
        case seekFromCurrentPosition:
            if ((0 < offset && position_ < -offset) || offset > size - position_)
                return sysErrParamErr;
            position_ += offset;
            break;
        
        default:
            assert(false);
            return sysErrParamErr;    
    }
    return errNone;
}

status_t BufferedReader::readRaw(void* buffer, uint_t& length) 
{
    if (isMarked_)
        return readMarked(buffer, length);
        
    return readNonMarked(buffer, length);
}

status_t BufferedReader::readNonMarked(void* buffer, uint_t& length)
{
    Buffer_t::size_type size = buffer_.size();
    assert(size >= position_);
    uint_t lengthLeft = length;
    length = std::min<uint_t>(size - position_, lengthLeft);
    std::memcpy(buffer, &buffer_[position_], length);
    position_ += length;
    lengthLeft -= length;
    if (0 == lengthLeft)
        return errNone;
    buffer = static_cast<char*>(buffer) + length;

    uint_t overBuffer = lengthLeft % chunkSize_;
    uint_t directReadLength = lengthLeft - overBuffer;
    status_t error = reader_.readRaw(buffer, directReadLength);
    if (errNone != error)
        return error;
    length += directReadLength;        
    if (directReadLength < lengthLeft - overBuffer)
        return errNone;
    lengthLeft -= directReadLength;
    if (0 == lengthLeft)
        return errNone;
    buffer = static_cast<char*>(buffer) + directReadLength;

    assert(size == position_);
    position_ = 0;
    buffer_.resize(chunkSize_);
    uint_t lastPartLength = chunkSize_;
    error = reader_.readRaw(&buffer_[position_], lastPartLength);
    if (errNone != error)
    {
        buffer_.clear();
        return error;
    }
    buffer_.resize(lastPartLength);
    overBuffer = std::min(overBuffer, lastPartLength);
    std::memcpy(buffer, &buffer_[position_], overBuffer);
    position_ += overBuffer;
    length += overBuffer;
    return errNone;
}

status_t BufferedReader::readMarked(void* buffer, uint_t& length)
{
    Buffer_t::size_type size = buffer_.size();
    assert(size >= position_);
    uint_t lengthLeft = length;
    length = std::min<uint_t>(size - position_, length);
    std::memcpy(buffer, &buffer_[position_], length);
    position_ += length;
    lengthLeft -= length;
    if (0 == lengthLeft)
        return errNone;
    buffer = static_cast<char*>(buffer) + length;
    
    buffer_.resize(size + lengthLeft);
    uint_t tail = lengthLeft;
    status_t error = reader_.readRaw(&buffer_[position_], tail);
    if (errNone != error)
    {
        buffer_.resize(size);
        return error;
    }
    buffer_.resize(size + tail);
    std::memcpy(buffer, &buffer_[position_], tail);
    length += tail;
    position_ += tail;
    return errNone;
}

status_t BufferedReader::readLine(bool& eof, String& out, char_t delimiter)
{
    String line;
    while (true) 
    {
        const char_t* begin = (const char_t*)&buffer_[position_];
        uint_t length = (buffer_.size() - position_)/sizeof(char_t);
        if (0 != length)
        {
            const char_t* end = begin + length;
            const char_t* pos = std::find(begin, end, delimiter);
            uint_t readLength = pos - begin;
            line.append(begin, readLength);
            position_ += readLength * sizeof(char_t);
            if (pos != end) // We've found delimiter
            {
                position_ += sizeof(char_t); // Skip delimiter
                break;
            }
        }
        assert(buffer_.size() == position_);
        position_ = 0;
        buffer_.resize(chunkSize_);
        length = chunkSize_;
        status_t error = reader_.readRaw(&buffer_[position_], length);
        if (errNone != error)
        {
            buffer_.clear();
            return error;
        }
        buffer_.resize(length);
        if (0 == length)
        {
            eof = true;
            break;
        }
    }
    out = line;
    return errNone;
}


    

