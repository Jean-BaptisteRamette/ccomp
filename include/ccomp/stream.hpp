#ifndef CCOMP_STREAM_INTERFACE_HPP
#define CCOMP_STREAM_INTERFACE_HPP


#include <string_view>
#include <fstream>


namespace ccomp
{
    class stream
    {
    public:
        explicit stream(std::ifstream& src_stream);
        explicit stream(std::string&& src_stream);

        bool eof() const;
        char get();
        char peek() const;
        void seek(size_t offset);
        size_t tellg() const;
        void   unget();
        std::string substr(size_t beg, size_t size) const;

    private:
        std::string buffer;

        size_t read {};
    };
}


#endif //CCOMP_STREAM_INTERFACE_HPP
