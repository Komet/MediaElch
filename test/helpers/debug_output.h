#pragma once

#include "data/Actor.h"
#include "data/tv_show/EpisodeNumber.h"
#include "data/tv_show/SeasonNumber.h"

#include <QDate>
#include <QString>
#include <QUrl>
#include <ostream>

inline std::ostream& operator<<(std::ostream& os, const QByteArray& value)
{
    return os << '"' << (value.isEmpty() ? "" : value.constData()) << '"';
}

inline std::ostream& operator<<(std::ostream& os, const QLatin1String& value)
{
    return os << '"' << value.latin1() << '"';
}

inline std::ostream& operator<<(std::ostream& os, const QString& value)
{
    return os << value.toLocal8Bit();
}

inline std::ostream& operator<<(std::ostream& os, const QDate& value)
{
    return os << value.toString("yyyy-MM-dd");
}

inline std::ostream& operator<<(std::ostream& os, const QUrl& value)
{
    return os << value.toDisplayString();
}

inline std::ostream& operator<<(std::ostream& os, const EpisodeNumber& value)
{
    return os << value.toString();
}

inline std::ostream& operator<<(std::ostream& os, const SeasonNumber& value)
{
    return os << value.toString();
}

inline std::ostream& operator<<(std::ostream& os, const QVector<Actor*>& value)
{
    return os << "vector(" << value.size() << " actors)";
}
