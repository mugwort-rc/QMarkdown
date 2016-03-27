#ifndef PYPP_HPP_
#define PYPP_HPP_

#include <algorithm>
#include <exception>
#include <functional>

#include <QDebug>
#include <QRegularExpressionMatchIterator>
#include <QString>

namespace pypp {

class Exception : public std::exception
{};

class IndexError : public Exception
{};

template <typename T>
inline QList<T> reversed(const QList<T> &in) {
    QList<T> result;
    result.reserve(in.size()); // reserve is new in Qt 4.7
    std::reverse_copy(in.begin(), in.end(), std::back_inserter(result));
    return result;
}

typedef QString str;

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

namespace re {

inline pypp::str sub(const QRegularExpression &pattern, const std::function<pypp::str(const QRegularExpressionMatch &)> &repl, const pypp::str &string)
{
    QStringList temp;
    int before = 0;
    QRegularExpressionMatchIterator it = pattern.globalMatch(string);
    if ( ! it.hasNext() ) {
        return string;
    }
    while ( it.hasNext() ) {
        QRegularExpressionMatch match = it.next();
        temp.append(string.mid(before, match.capturedStart()-before));
        before = match.capturedEnd();
        temp.append(repl(match));
    }
    return temp.join(QString());
}

} // namespace re

} // namespace pypp

#endif // PYPP_HPP_

