#pragma once

#include <QHash>
#include <QString>
#include <ostream>

class EpisodeNumber
{
public:
    EpisodeNumber() = default;
    explicit EpisodeNumber(int episodeNumber) noexcept;

    bool operator==(const EpisodeNumber& other) const;
    bool operator!=(const EpisodeNumber& other) const;
    bool operator>(const EpisodeNumber& other) const;
    bool operator<(const EpisodeNumber& other) const;

    int toInt() const;
    QString toPaddedString() const;
    QString toString() const;

    static const EpisodeNumber NoEpisode;

private:
    int m_episodeNumber = -1; // No episode
};

inline uint qHash(const EpisodeNumber& episode, uint seed)
{
    return qHash(episode.toInt(), seed);
}

std::ostream& operator<<(std::ostream& os, const EpisodeNumber& id);
QDebug operator<<(QDebug debug, const EpisodeNumber& season);
