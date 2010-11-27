#include "pch.h"

#include "StackStream.h"

namespace nexus {

StackStream::StackStream(char * begin, size_t size)
    : begin_(begin), pos_(begin), end_(begin + size) {}

void StackStream::put(const char * str)
{
    ptrdiff_t n = strnlen(str, end_ - pos_ + 1);
    checkOverflow(n);
    memcpy(pos_, str, n);
    pos_ += n;
}

void StackStream::put(const std::string & str)
{
    ptrdiff_t n = str.length();
    checkOverflow(n);
    memcpy(pos_, str.c_str(), n);
    pos_ += n;
}

void StackStream::writeCString(const std::string & str)
{
    ptrdiff_t n = str.length() + 1;
    checkOverflow(n);
    memcpy(pos_, str.c_str(), n);
    pos_ += n;
}

void StackStream::put(char ch)
{
    checkOverflow(1);
    *pos_ = ch;
    ++pos_;
}

void StackStream::write(const char * data, size_t n)
{
    checkOverflow(n);
    memcpy(pos_, data, n);
    pos_ += n;
}

Buffer StackStream::buffer() const
{
    return Buffer(begin_, pos_);
}

void StackStream::checkOverflow(size_t required)
{
    if(static_cast<size_t>(end_ - pos_) < required)
        BOOST_THROW_EXCEPTION(StackStreamException() << mstd::error_message("stack buffer overflow") << ErrorPosition(pos_ - begin_) << ErrorSize(end_ - begin_) << ErrorRequired(required));
}

}
