#include <ccomp/stream.hpp>


namespace ccomp
{
    stream::stream(std::ifstream& src_stream) : buffer(std::istreambuf_iterator<char>(src_stream), std::istreambuf_iterator<char>()), read {0 } {}

    stream::stream(std::string&& src_stream) : buffer(std::move(src_stream)), read {0 } {}

    bool stream::eof() const
    {
        return read >= buffer.size();
    }

	char stream::peek() const
	{
		if (eof())
			return 0;

		return buffer[read];
	}

    char stream::get()
    {
        if (eof())
            return 0;

        return buffer[read++];
    }

    void stream::unget()
    {
        if (read > 0)
            --read;
    }
}
