#include "Filter.h"

/**
 * @brief Filter::Filter
 * @param text Text displayed in the list of filters
 * @param shortText Text displayed in the filter input
 * @param filterText List of strings the filter responds to
 * @param info Filter type
 * @param hasInfo Info should be there or not
 */
Filter::Filter(QString text, QString shortText, QStringList filterText, int info, bool hasInfo, int data)
{
    m_text = text;
    m_shortText = shortText;
    m_filterText = filterText;
    m_info = info;
    m_hasInfo = hasInfo;
    m_data = data;
}

/**
 * @brief Checks if a filter responds to the given text
 * @param text Text to check
 * @return True if the filter responds to the given text
 */
bool Filter::accepts(QString text) const
{
    if (m_info == MovieFilters::Title || (m_info == MovieFilters::ImdbId && m_hasInfo) || m_info == MovieFilters::Path || m_info == TvShowFilters::Title || m_info == ConcertFilters::Title)
        return true;
    foreach (const QString &filterText, m_filterText) {
        if (filterText.startsWith(text, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

/**
 * @brief Holds the text
 * @return Text
 */
QString Filter::text() const
{
    return m_text;
}

bool Filter::hasInfo() const
{
    return m_hasInfo;
}

/**
 * @brief Sets the filter text
 * @param text Text to set
 */
void Filter::setText(QString text)
{
    m_text = text;
}

/**
 * @brief This property holds the short text which is displayed in the filter line edit
 * @param shortText Text to set
 */
void Filter::setShortText(QString shortText)
{
    m_shortText = shortText;
}

/**
 * @brief This property holds the short text which is displayed in the filter line edit
 * @return short text
 */
QString Filter::shortText() const
{
    return m_shortText;
}

/**
 * @brief This property holds the filter type
 * @return Filter type
 */
int Filter::info() const
{
    return m_info;
}

/**
 * @brief Checks if the filter accepts a movie object
 * @param movie Movie to check
 * @return True if the movie object is accepted
 */
bool Filter::accepts(Movie *movie)
{
    if (m_info == MovieFilters::Poster)
        return (m_hasInfo && movie->hasImage(ImageType::MoviePoster)) || (!m_hasInfo && !movie->hasImage(ImageType::MoviePoster));
    if (m_info == MovieFilters::Backdrop)
        return (m_hasInfo && movie->hasImage(ImageType::MovieBackdrop)) || (!m_hasInfo && !movie->hasImage(ImageType::MovieBackdrop));
    if (m_info == MovieFilters::ExtraFanarts)
        return (m_hasInfo && movie->hasExtraFanarts()) || (!m_hasInfo && !movie->hasExtraFanarts());
    if (m_info == MovieFilters::Actors)
        return (m_hasInfo && !movie->actors().isEmpty()) || (!m_hasInfo && movie->actors().isEmpty());
    if (m_info == MovieFilters::Logo)
        return (m_hasInfo && movie->hasImage(ImageType::MovieLogo)) || (!m_hasInfo && !movie->hasImage(ImageType::MovieLogo));
    if (m_info == MovieFilters::ClearArt)
        return (m_hasInfo && movie->hasImage(ImageType::MovieClearArt)) || (!m_hasInfo && !movie->hasImage(ImageType::MovieClearArt));
    if (m_info == MovieFilters::Banner)
        return (m_hasInfo && movie->hasImage(ImageType::MovieBanner)) || (!m_hasInfo && !movie->hasImage(ImageType::MovieBanner));
    if (m_info == MovieFilters::Thumb)
        return (m_hasInfo && movie->hasImage(ImageType::MovieThumb)) || (!m_hasInfo && !movie->hasImage(ImageType::MovieThumb));
    if (m_info == MovieFilters::CdArt)
        return (m_hasInfo && movie->hasImage(ImageType::MovieCdArt)) || (!m_hasInfo && !movie->hasImage(ImageType::MovieCdArt));
    if (m_info == MovieFilters::Trailer)
        return (m_hasInfo && !movie->trailer().isEmpty()) || (!m_hasInfo && movie->trailer().isEmpty());
    if (m_info == MovieFilters::LocalTrailer)
        return (m_hasInfo && movie->hasLocalTrailer()) || (!m_hasInfo && !movie->hasLocalTrailer());
    if (m_info == MovieFilters::Certification)
        return (m_hasInfo && movie->certification() == m_shortText) || (!m_hasInfo && movie->certification().isEmpty());
    if (m_info == MovieFilters::Genres)
        return (m_hasInfo && movie->genres().contains(m_shortText)) || (!m_hasInfo && movie->genres().isEmpty());
    if (m_info == MovieFilters::Released)
        return movie->released().isValid() && movie->released().year() == m_shortText.toInt();
    if (m_info == MovieFilters::Watched)
        return (m_hasInfo && movie->watched()) || (!m_hasInfo && !movie->watched());
    if (m_info == MovieFilters::Title)
        return movie->name().contains(m_shortText, Qt::CaseInsensitive);
    if (m_info == MovieFilters::StreamDetails)
        return (m_hasInfo && movie->streamDetailsLoaded()) || (!m_hasInfo && !movie->streamDetailsLoaded());
    if (m_info == MovieFilters::Studio)
        return (m_hasInfo && movie->studios().contains(m_shortText)) || (!m_hasInfo && movie->studios().isEmpty());
    if (m_info == MovieFilters::Set)
        return (m_hasInfo && movie->set() == m_shortText) || (!m_hasInfo && movie->set().isEmpty());
    if (m_info == MovieFilters::Country)
        return (m_hasInfo && movie->countries().contains(m_shortText)) || (!m_hasInfo && movie->countries().isEmpty());
    if (m_info == MovieFilters::Tags)
        return (m_hasInfo && movie->tags().contains(m_shortText)) || (!m_hasInfo && movie->tags().isEmpty());
    if (m_info == MovieFilters::Director)
        return (m_hasInfo && movie->director() == m_shortText) || (!m_hasInfo && movie->director().isEmpty());
    if (m_info == MovieFilters::ImdbId)
        return (m_hasInfo && movie->id() == m_shortText) || (!m_hasInfo && movie->id().isEmpty());
    if (m_info == MovieFilters::Rating)
        return (m_hasInfo && movie->rating() != 0) || (!m_hasInfo && movie->rating() == 0);
    if (m_info == MovieFilters::HasExternalSubtitle)
        return (m_hasInfo && !movie->subtitles().isEmpty()) || (!m_hasInfo && movie->subtitles().isEmpty());
    if (m_info == MovieFilters::HasSubtitle)
        return (m_hasInfo && (!movie->subtitles().isEmpty() || !movie->streamDetails()->subtitleDetails().isEmpty())) || (!m_hasInfo && movie->subtitles().isEmpty() && movie->streamDetails()->subtitleDetails().isEmpty());

    if (m_info == MovieFilters::Quality) {
        if (m_shortText == "1080p")
            return movie->streamDetails()->videoDetails().value("width").toInt() == 1920;
        else if (m_shortText == "720p")
            return movie->streamDetails()->videoDetails().value("width").toInt() == 1280;
        else if (m_shortText == "SD")
            return movie->streamDetails()->videoDetails().value("width").toInt() > 0 && movie->streamDetails()->videoDetails().value("width").toInt() <= 720;
        else if (m_shortText == "BluRay")
            return movie->discType() == DiscBluRay;
        else if (m_shortText == "DVD")
            return movie->discType() == DiscDvd;
    }

    if (m_info == MovieFilters::AudioChannels) {
        if (m_shortText == "2.0")
            return movie->streamDetails()->hasAudioChannels(2);
        else if (m_shortText == "5.1")
            return movie->streamDetails()->hasAudioChannels(6);
        else if (m_shortText == "7.1")
            return movie->streamDetails()->hasAudioChannels(8);
    }

    if (m_info == MovieFilters::AudioQuality) {
        if (m_shortText == "HD Audio")
            return movie->streamDetails()->hasAudioQuality("hd");
        if (m_shortText == "Normal Audio")
            return movie->streamDetails()->hasAudioQuality("normal");
        if (m_shortText == "SD Audio")
            return movie->streamDetails()->hasAudioQuality("sd");
    }

    if (m_info == MovieFilters::Path) {
        foreach (const QString &file, movie->files()) {
            if (file.contains(m_shortText, Qt::CaseInsensitive))
                return true;
        }
        return false;
    }

    if (m_info == MovieFilters::Label)
        return movie->label() == m_data;

    return true;
}

/**
 * @brief Checks if the filter accepts a tv show object
 * @param show Tv Show to check
 * @return True if the tv show object is accepted
 */
bool Filter::accepts(TvShow *show)
{
    if (m_info == TvShowFilters::Title)
        return show->name().contains(m_shortText, Qt::CaseInsensitive);
    return true;
}

bool Filter::accepts(TvShowEpisode *episode)
{
    if (m_info == TvShowFilters::Title)
        return episode->name().contains(m_shortText, Qt::CaseInsensitive);
    return true;
}

/**
 * @brief Checks if the filter accepts a concert object
 * @param concert Concert to check
 * @return True if the concert object is accepted
 */
bool Filter::accepts(Concert *concert)
{
    if (m_info == ConcertFilters::Title)
        return concert->name().contains(m_shortText, Qt::CaseInsensitive);
    return true;
}
