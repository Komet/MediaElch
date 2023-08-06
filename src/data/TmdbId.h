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

    /// \brief   Returns true if the given id has the common TheTvDb ID format.
    /// \details A TmdbId ID is valid if it starts with "tmdb".
    static bool isValidPrefixedFormat(const QString& tmdbId);
    /// \brief   Returns true if the given id has a common TheTvDb ID format.
    /// \details A TmdbId ID is valid if it starts with "tmdb", but this function
    ///          accepts a number without the prefix as well..
    static bool isValidFormat(const QString& tmdbId);
    static QString removePrefix(const QString& tmdbId);

    static const TmdbId NoId;

private:
    QString m_tmdbId;
};

std::ostream& operator<<(std::ostream& os, const TmdbId& id);
QDebug operator<<(QDebug debug, const TmdbId& id);
