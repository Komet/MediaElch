#include "Filter.h"

#include <utility>

#include "concerts/Concert.h"
#include "movies/Movie.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

/**
 * \brief Filter::Filter
 * \param text Text displayed in the list of filters
 * \param shortText Text displayed in the filter input
 * \param filterText List of strings the filter responds to
 * \param info Filter type
 * \param hasInfo Info should be there or not
 */
Filter::Filter(QString text,
    QString shortText,
    QStringList filterText,
    MovieFilters info,
    bool hasInfo,
    ColorLabel data) :
    Filter(text, shortText, filterText, hasInfo, data)
{
    m_movieInfo = info;
    m_type = FilterType::Movie;
}

Filter::Filter(QString text,
    QString shortText,
    QStringList filterText,
    MusicFilters info,
    bool hasInfo,
    ColorLabel data) :
    Filter(text, shortText, filterText, hasInfo, data)
{
    m_musicInfo = info;
    m_type = FilterType::Music;
}

Filter::Filter(QString text,
    QString shortText,
    QStringList filterText,
    ConcertFilters info,
    bool hasInfo,
    ColorLabel data) :
    Filter(text, shortText, filterText, hasInfo, data)
{
    m_concertInfo = info;
    m_type = FilterType::Concert;
}

Filter::Filter(QString text,
    QString shortText,
    QStringList filterText,
    TvShowFilters info,
    bool hasInfo,
    ColorLabel data) :
    Filter(text, shortText, filterText, hasInfo, data)
{
    m_type = FilterType::TvShow;
    m_showInfo = info;
}

Filter::Filter(QString text, QString shortText, QStringList filterText, bool hasInfo, ColorLabel data) :
    m_text{std::move(text)},
    m_shortText{std::move(shortText)},
    m_filterText{std::move(filterText)},
    m_data{data},
    m_hasInfo{hasInfo}
{
}

/**
 * \brief Checks if a filter responds to the given text, e.g. whether we should display the filter type in drop-downs.
 * \param text Text to check
 * \return True if the filter responds to the given text
 */
bool Filter::accepts(QString text) const
{
    if (isInfo(MovieFilters::Title) || isInfo(MovieFilters::OriginalTitle)
        || (isInfo(MovieFilters::ImdbId) && m_hasInfo) || (isInfo(MovieFilters::TmdbId) && m_hasInfo)
        || isInfo(MovieFilters::Path) || isInfo(TvShowFilters::Title) || isInfo(ConcertFilters::Title)) {
        return true;
    }
    for (const auto& filterText : m_filterText) {
        if (filterText.startsWith(text, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

/**
 * \brief Holds the text
 * \return Text
 */
QString Filter::text() const
{
    return m_text;
}

bool Filter::hasInfo() const
{
    return m_hasInfo;
}

bool Filter::isInfo(MovieFilters info) const
{
    return m_type == FilterType::Movie && m_movieInfo == info;
}

bool Filter::isInfo(TvShowFilters info) const
{
    return m_type == FilterType::TvShow && m_showInfo == info;
}

bool Filter::isInfo(MusicFilters info) const
{
    return m_type == FilterType::Music && m_musicInfo == info;
}

bool Filter::isInfo(ConcertFilters info) const
{
    return m_type == FilterType::Concert && m_concertInfo == info;
}

/**
 * \brief Sets the filter text
 * \param text Text to set
 */
void Filter::setText(QString text)
{
    m_text = text;
}

/**
 * \brief This property holds the short text which is displayed in the filter line edit
 * \param shortText Text to set
 */
void Filter::setShortText(QString shortText)
{
    m_shortText = shortText;
}

/**
 * \brief This property holds the short text which is displayed in the filter line edit
 * \return short text
 */
QString Filter::shortText() const
{
    return m_shortText;
}

/**
 * \brief Checks if the filter accepts a movie object
 * \param movie Movie to check
 * \return True if the movie object is accepted
 */
bool Filter::accepts(Movie* movie)
{
    if (isInfo(MovieFilters::Poster)) {
        return (m_hasInfo && movie->hasImage(ImageType::MoviePoster))
               || (!m_hasInfo && !movie->hasImage(ImageType::MoviePoster));
    }
    if (isInfo(MovieFilters::Backdrop)) {
        return (m_hasInfo && movie->hasImage(ImageType::MovieBackdrop))
               || (!m_hasInfo && !movie->hasImage(ImageType::MovieBackdrop));
    }
    if (isInfo(MovieFilters::ExtraFanarts)) {
        return (m_hasInfo && movie->images().hasExtraFanarts()) || (!m_hasInfo && !movie->images().hasExtraFanarts());
    }
    if (isInfo(MovieFilters::Actors)) {
        return (m_hasInfo && !movie->actors().isEmpty()) || (!m_hasInfo && movie->actors().isEmpty());
    }
    if (isInfo(MovieFilters::Logo)) {
        return (m_hasInfo && movie->hasImage(ImageType::MovieLogo))
               || (!m_hasInfo && !movie->hasImage(ImageType::MovieLogo));
    }
    if (isInfo(MovieFilters::ClearArt)) {
        return (m_hasInfo && movie->hasImage(ImageType::MovieClearArt))
               || (!m_hasInfo && !movie->hasImage(ImageType::MovieClearArt));
    }
    if (isInfo(MovieFilters::Banner)) {
        return (m_hasInfo && movie->hasImage(ImageType::MovieBanner))
               || (!m_hasInfo && !movie->hasImage(ImageType::MovieBanner));
    }
    if (isInfo(MovieFilters::Thumb)) {
        return (m_hasInfo && movie->hasImage(ImageType::MovieThumb))
               || (!m_hasInfo && !movie->hasImage(ImageType::MovieThumb));
    }
    if (isInfo(MovieFilters::CdArt)) {
        return (m_hasInfo && movie->hasImage(ImageType::MovieCdArt))
               || (!m_hasInfo && !movie->hasImage(ImageType::MovieCdArt));
    }
    if (isInfo(MovieFilters::Trailer)) {
        return (m_hasInfo && !movie->trailer().isEmpty()) || (!m_hasInfo && movie->trailer().isEmpty());
    }
    if (isInfo(MovieFilters::LocalTrailer)) {
        return (m_hasInfo && movie->hasLocalTrailer()) || (!m_hasInfo && !movie->hasLocalTrailer());
    }
    if (isInfo(MovieFilters::Certification)) {
        return (m_hasInfo && movie->certification().toString() == m_shortText)
               || (!m_hasInfo && !movie->certification().isValid());
    }
    if (isInfo(MovieFilters::Genres)) {
        return (m_hasInfo && movie->genres().contains(m_shortText)) || (!m_hasInfo && movie->genres().isEmpty());
    }
    if (isInfo(MovieFilters::Released)) {
        return movie->released().isValid() && movie->released().year() == m_shortText.toInt();
    }
    if (isInfo(MovieFilters::Watched)) {
        return (m_hasInfo && movie->watched()) || (!m_hasInfo && !movie->watched());
    }
    if (isInfo(MovieFilters::Title)) {
        return movie->name().contains(m_shortText, Qt::CaseInsensitive);
    }
    if (isInfo(MovieFilters::OriginalTitle)) {
        return movie->originalName().contains(m_shortText, Qt::CaseInsensitive);
    }
    if (isInfo(MovieFilters::StreamDetails)) {
        return (m_hasInfo && movie->streamDetailsLoaded()) || (!m_hasInfo && !movie->streamDetailsLoaded());
    }
    if (isInfo(MovieFilters::Studio)) {
        return (m_hasInfo && movie->studios().contains(m_shortText)) || (!m_hasInfo && movie->studios().isEmpty());
    }
    if (isInfo(MovieFilters::Set)) {
        return (m_hasInfo && movie->set().name == m_shortText) || (!m_hasInfo && movie->set().name.isEmpty());
    }
    if (isInfo(MovieFilters::Country)) {
        return (m_hasInfo && movie->countries().contains(m_shortText)) || (!m_hasInfo && movie->countries().isEmpty());
    }
    if (isInfo(MovieFilters::Tags)) {
        return (m_hasInfo && movie->tags().contains(m_shortText)) || (!m_hasInfo && movie->tags().isEmpty());
    }
    if (isInfo(MovieFilters::Director)) {
        return (m_hasInfo && movie->director() == m_shortText) || (!m_hasInfo && movie->director().isEmpty());
    }
    if (isInfo(MovieFilters::VideoCodec)) {
        return (m_hasInfo
                   && movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::Codec) == m_shortText)
               || (!m_hasInfo
                   && movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::Codec).isEmpty());
    }
    if (isInfo(MovieFilters::ImdbId)) {
        return (m_hasInfo && movie->imdbId() == ImdbId(m_shortText)) || (!m_hasInfo && !movie->imdbId().isValid());
    }
    if (isInfo(MovieFilters::TmdbId)) {
        return (m_hasInfo && movie->tmdbId() == TmdbId(m_shortText)) || (!m_hasInfo && !movie->tmdbId().isValid());
    }
    if (isInfo(MovieFilters::Rating)) {
        return (m_hasInfo && !movie->ratings().isEmpty()) || (!m_hasInfo && movie->ratings().isEmpty());
    }
    if (isInfo(MovieFilters::HasExternalSubtitle)) {
        return (m_hasInfo && !movie->subtitles().isEmpty()) || (!m_hasInfo && movie->subtitles().isEmpty());
    }
    if (isInfo(MovieFilters::HasSubtitle)) {
        return (m_hasInfo && (!movie->subtitles().isEmpty() || !movie->streamDetails()->subtitleDetails().isEmpty()))
               || (!m_hasInfo && movie->subtitles().isEmpty() && movie->streamDetails()->subtitleDetails().isEmpty());
    }

    if (isInfo(MovieFilters::Quality)) {
        const int width = movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::Width).toInt();
        if (m_shortText == "2160p") {
            return width == 3840;
        }
        if (m_shortText == "1080p") {
            return width == 1920;
        }
        if (m_shortText == "720p") {
            return width == 1280;
        }
        if (m_shortText == "SD") {
            return width > 0 && width <= 720;
        }
        if (m_shortText == "BluRay") {
            return movie->discType() == DiscType::BluRay;
        }
        if (m_shortText == "DVD") {
            return movie->discType() == DiscType::Dvd;
        }
    }

    if (isInfo(MovieFilters::AudioChannels)) {
        if (m_shortText == "2.0") {
            return movie->streamDetails()->hasAudioChannels(2);
        }
        if (m_shortText == "5.1") {
            return movie->streamDetails()->hasAudioChannels(6);
        }
        if (m_shortText == "7.1") {
            return movie->streamDetails()->hasAudioChannels(8);
        }
    }

    if (isInfo(MovieFilters::AudioQuality)) {
        if (m_shortText == "HD Audio") {
            return movie->streamDetails()->hasAudioQuality("hd");
        }
        if (m_shortText == "Normal Audio") {
            return movie->streamDetails()->hasAudioQuality("normal");
        }
        if (m_shortText == "SD Audio") {
            return movie->streamDetails()->hasAudioQuality("sd");
        }
    }

    if (isInfo(MovieFilters::Path)) {
        for (const mediaelch::FilePath& file : movie->files()) {
            if (file.toNativePathString().contains(m_shortText, Qt::CaseInsensitive)) {
                return true;
            }
        }
        return false;
    }

    if (isInfo(MovieFilters::Label)) {
        return movie->label() == m_data;
    }

    return true;
}

/**
 * \brief Checks if the filter accepts a TV show object
 * \param show Tv Show to check
 * \return True if the TV show object is accepted
 */
bool Filter::accepts(TvShow* show)
{
    if (isInfo(TvShowFilters::Title)) {
        return show->title().contains(m_shortText, Qt::CaseInsensitive);
    }
    return true;
}

bool Filter::accepts(TvShowEpisode* episode)
{
    if (isInfo(TvShowFilters::Title)) {
        return episode->title().contains(m_shortText, Qt::CaseInsensitive);
    }
    return true;
}

/**
 * \brief Checks if the filter accepts a concert object
 * \param concert Concert to check
 * \return True if the concert object is accepted
 */
bool Filter::accepts(Concert* concert)
{
    if (isInfo(ConcertFilters::Title)) {
        return concert->title().contains(m_shortText, Qt::CaseInsensitive);
    }
    return true;
}
