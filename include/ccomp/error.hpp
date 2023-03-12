#ifndef CCOMP_ERROR_HPP
#define CCOMP_ERROR_HPP

#include <stdexcept>


namespace ccomp
{
    enum class error_code
    {
        ok,
        io_err,
        file_not_found_err
    };
}

#endif //CCOMP_ERROR_HPP
