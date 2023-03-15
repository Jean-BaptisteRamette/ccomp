#include <ccomp/stream.hpp>


namespace ccomp
{
    stream::stream(std::ifstream& src_stream) : buffer(std::istreambuf_iterator<char>(src_stream), std::istreambuf_iterator<char>()), cursor { 0 } {}

    stream::stream(std::string&& src_stream) : buffer(std::move(src_stream)), cursor { 0 } {}

    bool stream::eof() const
    {
        return cursor >= buffer.size();
    }

    char stream::get()
    {
        if (eof())
            return 0;

        return buffer[cursor++];
    }

    void stream::unget()
    {
        if (cursor > 0)
            --cursor;
    }

    char stream::peek() const
    {
        if (eof())
            return 0;

        return buffer[cursor];
    }

    size_t stream::tellg() const
    {
        return cursor;
    }


    void stream::seek(size_t offset)
    {
        if (offset >= buffer.size())
            throw std::runtime_error("stream.cpp: stream::seek offset >= buffer.size()\n");

        cursor = offset;
    }

    std::string_view stream::substr(size_t beg, size_t size) const
    {
        return { &buffer[beg], size };
    }
}
