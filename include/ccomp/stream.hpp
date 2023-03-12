#ifndef CCOMP_STREAM_INTERFACE_HPP
#define CCOMP_STREAM_INTERFACE_HPP


#include <fstream>


namespace ccomp
{
    class stream
    {
    public:
        explicit stream(std::ifstream& src_stream);
        explicit stream(std::string&& src_stream);

        bool eof();
        char get();
        char peek();
        void seek(size_t offset);
        size_t tellg();
        const char* iterator();

    private:
        std::string buffer;

        size_t cursor {};
    };
}


#endif //CCOMP_STREAM_INTERFACE_HPP
