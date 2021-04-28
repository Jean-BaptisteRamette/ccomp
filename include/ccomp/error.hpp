#ifndef CCOMP_ERROR_HPP
#define CCOMP_ERROR_HPP

#include <stdexcept>

namespace ccomp
{
    struct error : std::runtime_error
    {

    };
}

#endif //CCOMP_ERROR_HPP
