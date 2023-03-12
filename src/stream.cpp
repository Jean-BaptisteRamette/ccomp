#include <ccomp/stream.hpp>


namespace ccomp
{
    stream::stream(std::ifstream& src_stream) : buffer(std::istreambuf_iterator<char>(src_stream), std::istreambuf_iterator<char>()), cursor { 0 } {}

    stream::stream(std::string&& src_stream) : buffer(std::move(src_stream)), cursor { 0 } {}

    bool stream::eof()
    {
        return cursor >= buffer.size();
    }

    char stream::get()
    {
        if (eof())
            return 0;

        return buffer[cursor++];
    }

    char stream::peek()
    {
        if (eof())
            return 0;

        return buffer[cursor];
    }

    void stream::seek(size_t offset)
    {
        if (offset >= buffer.size())
            throw std::runtime_error("stream.cpp: stream::seek offset >= buffer.size()\n");

        cursor = offset;
    }

    size_t stream::tellg()
    {
        return cursor;
    }

    const char* stream::iterator()
    {
        if (eof())
            return nullptr;

        return &buffer[cursor];
    }
}
