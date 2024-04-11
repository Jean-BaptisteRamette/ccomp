#ifndef CHASM_STREAM_INTERFACE_HPP
#define CHASM_STREAM_INTERFACE_HPP


#include <string_view>
#include <filesystem>
#include <fstream>


namespace chasm
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


#endif //CHASM_STREAM_INTERFACE_HPP
