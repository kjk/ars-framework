#include <BufferedReader.hpp>

using namespace ArsLexis;

BufferedReader::BufferedReader(Reader& reader, uint_t chunkSize):
    reader_(reader),
    isMarked_(false),
    position_(0),
    chunkSize_(chunkSize)
{}

BufferedReader::~BufferedReader() {}

void BufferedReader::mark()
{
    assert(position_ < buffer_.size());
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

void BufferedReader::rewind()
{
    assert(isMarked_);
    position_ = 0;
}

