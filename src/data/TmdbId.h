#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class TmdbId
{
public:
    TmdbId() = default;
    explicit TmdbId(QString tmdbId);
    explicit TmdbId(int tmdbId);

    bool operator==(const TmdbId& other) const;
    bool operator!=(const TmdbId& other) const;

    QString toString() const;
    QString withPrefix() const;
    bool isValid() const;

    static const TmdbId NoId;

private:
    QString m_tmdbId;
};

std::ostream& operator<<(std::ostream& os, const TmdbId& id);
QDebug operator<<(QDebug debug, const TmdbId& id);
