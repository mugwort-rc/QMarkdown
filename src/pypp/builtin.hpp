#ifndef PYPP_BUILTIN_HPP
#define PYPP_BUILTIN_HPP

#include <algorithm>

#include <QList>

namespace pypp {

template <typename T>
inline T reversed(const T &in) {
    T result;
    result.reserve(in.size()); // reserve is new in Qt 4.7
    std::reverse_copy(in.begin(), in.end(), std::back_inserter(result));
    return result;
}

}

#endif // PYPP_BUILTIN_HPP

