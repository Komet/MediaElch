#ifndef DATA_EPISODE_NUMBER
#define DATA_EPISODE_NUMBER

#include <QString>

class EpisodeNumber
{
public:
    EpisodeNumber() = default;
    explicit EpisodeNumber(int episodeNumber);

    bool operator==(const EpisodeNumber &other);
    bool operator!=(const EpisodeNumber &other);
    bool operator>(const EpisodeNumber &other);
    bool operator<(const EpisodeNumber &other);

    int toInt() const;
    QString toPaddedString() const;
    QString toString() const;

    static const EpisodeNumber NoEpisode;

private:
    int m_episodeNumber;
};

#endif // DATA_EPISODE_NUMBER
