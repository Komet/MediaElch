#ifndef TEST_DEBUG_OUTPUT
#define TEST_DEBUG_OUTPUT

#include <QString>
#include <ostream>

inline std::ostream &operator<<(std::ostream &os, const QByteArray &value)
{
    return os << '"' << (value.isEmpty() ? "" : value.constData()) << '"';
}

inline std::ostream &operator<<(std::ostream &os, const QLatin1String &value)
{
    return os << '"' << value.latin1() << '"';
}

inline std::ostream &operator<<(std::ostream &os, const QString &value)
{
    return os << value.toLocal8Bit();
}

#endif // TEST_DEBUG_OUTPUT
