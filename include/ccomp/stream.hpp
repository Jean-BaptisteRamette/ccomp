#ifndef CCOMP_STREAM_INTERFACE_HPP
#define CCOMP_STREAM_INTERFACE_HPP


#include <string_view>
#include <filesystem>
#include <fstream>


namespace ccomp
{
    class stream
    {
    public:
        explicit stream(std::string&& src_stream);

        [[nodiscard]] bool eof() const;
		[[nodiscard]] char peek() const;
        char get();
        void unget();

    private:
        std::string buffer;
        size_t read {};
    };
}


#endif //CCOMP_STREAM_INTERFACE_HPP
