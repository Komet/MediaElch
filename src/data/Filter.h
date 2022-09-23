#pragma once

#include "globals/Globals.h"

#include <QObject>
#include <QString>
#include <QStringList>

class Concert;
class Movie;
class TvShow;
class TvShowEpisode;

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
    Filter(const Filter& other) = default;
    Filter(Filter&& other) = default;
    Filter& operator=(const Filter&) & = default;
    Filter& operator=(Filter&&) & = default;
    ~Filter() = default;

    bool accepts(QString text) const;
    bool accepts(Movie* movie);
    bool accepts(Concert* concert);
    bool accepts(TvShow* show);
    bool accepts(TvShowEpisode* episode);
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
    MovieFilters m_movieInfo = MovieFilters::Title;
    TvShowFilters m_showInfo = TvShowFilters::Title;
    MusicFilters m_musicInfo = MusicFilters::Title;
    ConcertFilters m_concertInfo = ConcertFilters::Title;
    ColorLabel m_data = ColorLabel::NoLabel;
    FilterType m_type = FilterType::Movie;
    bool m_hasInfo = false;
};

Q_DECLARE_METATYPE(Filter*)
