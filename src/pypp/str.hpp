#ifndef PYPP_STR_HPP
#define PYPP_STR_HPP

#include <QString>

namespace pypp {

typedef QString str;

inline pypp::str expandtabs(const pypp::str &in, int tabsize)
{
    pypp::str result;
    int n = 0;
    for ( const auto &ch : in ) {
        if ( n >= tabsize ) {
            n = 0;
        }
        if ( ch == '\t' ) {
            result += pypp::str(tabsize-n, ' ');
        } else {
            result += ch;
            if ( ch == '\n' ) {
                n = 0;
            } else {
                ++n;
            }
        }
    }
    return result;
}

inline pypp::str lstrip(const pypp::str &in)
{
    for (int i = 0; i < in.size(); ++i) {
        if ( ! in.at(i).isSpace() ) {
            return in.mid(i);
        }
    }
    return pypp::str();
}

inline pypp::str rstrip(const pypp::str &in)
{
    for (int i = in.size()-1; i >= 0; --i) {
        if ( ! in.at(i).isSpace() ) {
            return in.left(i + 1);
        }
    }
    return pypp::str();
}

inline pypp::str lstrip(const pypp::str &in, const std::function<bool(const QChar &)> &isSpace)
{
    for (int i = 0; i < in.size(); ++i) {
        if ( ! isSpace(in.at(i)) ) {
            return in.mid(i);
        }
    }
    return pypp::str();
}

inline pypp::str rstrip(const pypp::str &in, const std::function<bool(const QChar &)> &isSpace)
{
    for (int i = in.size()-1; i >= 0; --i) {
        if ( ! isSpace(in.at(i)) ) {
            return in.left(i + 1);
        }
    }
    return pypp::str();
}

} // namespace pypp

#endif // PYPP_STR_HPP

