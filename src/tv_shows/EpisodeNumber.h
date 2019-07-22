#pragma once

#include <QString>

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
