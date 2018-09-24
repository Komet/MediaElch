#ifndef TMDBID_H
#define TMDBID_H

#include <QString>

class TmdbId
{
public:
    TmdbId() = default;
    explicit TmdbId(QString tmdbId);
    explicit TmdbId(int tmdbId);

    bool operator==(const TmdbId &other);
    bool operator!=(const TmdbId &other);

    QString toString() const;
    QString withPrefix() const;
    bool isValid() const;

    static const TmdbId NoId;

private:
    QString m_tmdbId;
};

#endif // TMDBID_H
