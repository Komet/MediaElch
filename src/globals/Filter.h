#ifndef FILTER_H
#define FILTER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "data/Concert.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "globals/Globals.h"

/**
 * @brief The Filter class
 */
class Filter
{
public:
    Filter(QString text,
        QString shortText,
        QStringList filterText,
        MovieFilters info,
        bool hasInfo,
        ColorLabel data = ColorLabel::NoLabel);
    Filter(QString text,
        QString shortText,
        QStringList filterText,
        TvShowFilters info,
        bool hasInfo,
        ColorLabel data = ColorLabel::NoLabel);
    Filter(QString text,
        QString shortText,
        QStringList filterText,
        MusicFilters info,
        bool hasInfo,
        ColorLabel data = ColorLabel::NoLabel);
    Filter(QString text,
        QString shortText,
        QStringList filterText,
        ConcertFilters info,
        bool hasInfo,
        ColorLabel data = ColorLabel::NoLabel);

    // Rule of five
    Filter(const Filter &other) = default;
    Filter(Filter &&other) = default;
    Filter &operator=(const Filter &) & = default;
    Filter &operator=(Filter &&) & = default;
    ~Filter() = default;

    bool accepts(QString text) const;
    bool accepts(Movie *movie);
    bool accepts(Concert *concert);
    bool accepts(TvShow *show);
    bool accepts(TvShowEpisode *episode);
    QString text() const;
    QString shortText() const;
    void setShortText(QString shortText);
    void setText(QString text);
    bool hasInfo() const;

    bool isInfo(MovieFilters info) const;
    bool isInfo(TvShowFilters info) const;
    bool isInfo(MusicFilters info) const;
    bool isInfo(ConcertFilters info) const;

private:
    enum class FilterType
    {
        Movie,
        TvShow,
        Music,
        Concert
    };

    Filter(QString text,
        QString shortText,
        QStringList filterText,
        bool hasInfo,
        ColorLabel data = ColorLabel::NoLabel);

    QString m_text;
    QString m_shortText;
    QStringList m_filterText;
    MovieFilters m_movieInfo;
    TvShowFilters m_showInfo;
    MusicFilters m_musicInfo;
    ConcertFilters m_concertInfo;
    ColorLabel m_data;
    FilterType m_type;
    bool m_hasInfo;
};

Q_DECLARE_METATYPE(Filter *)

#endif // FILTER_H
