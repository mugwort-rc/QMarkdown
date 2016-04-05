#ifndef PYPP_EXCEPTIONS_HPP
#define PYPP_EXCEPTIONS_HPP

#include <exception>

namespace pypp {

class Exception : public std::exception
{};

class IndexError : public Exception
{};

} // namespace pypp

#endif // EXCEPTIONS_HPP

