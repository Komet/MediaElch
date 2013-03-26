#include "Filter.h"

/**
 * @brief Filter::Filter
 * @param text Text displayed in the list of filters
 * @param shortText Text displayed in the filter input
 * @param filterText List of strings the filter responds to
 * @param info Filter type
 * @param hasInfo Info should be there or not
 */
Filter::Filter(QString text, QString shortText, QStringList filterText, int info, bool hasInfo)
{
    m_text = text;
    m_shortText = shortText;
    m_filterText = filterText;
    m_info = info;
    m_hasInfo = hasInfo;
}

/**
 * @brief Checks if a filter responds to the given text
 * @param text Text to check
 * @return True if the filter responds to the given text
 */
bool Filter::accepts(QString text) const
{
    if (m_info == MovieFilters::Title || m_info == MovieFilters::Path || m_info == TvShowFilters::Title || m_info == ConcertFilters::Title)
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
        return (m_hasInfo && movie->hasPoster()) || (!m_hasInfo && !movie->hasPoster());
    if (m_info == MovieFilters::Backdrop)
        return (m_hasInfo && movie->hasBackdrop()) || (!m_hasInfo && !movie->hasBackdrop());
    if (m_info == MovieFilters::ExtraFanarts)
        return (m_hasInfo && movie->hasExtraFanarts()) || (!m_hasInfo && !movie->hasExtraFanarts());
    if (m_info == MovieFilters::Actors)
        return (m_hasInfo && !movie->actors().isEmpty()) || (!m_hasInfo && movie->actors().isEmpty());
    if (m_info == MovieFilters::Logo)
        return (m_hasInfo && movie->hasLogo()) || (!m_hasInfo && !movie->hasLogo());
    if (m_info == MovieFilters::ClearArt)
        return (m_hasInfo && movie->hasClearArt()) || (!m_hasInfo && !movie->hasClearArt());
    if (m_info == MovieFilters::CdArt)
        return (m_hasInfo && movie->hasCdArt()) || (!m_hasInfo && !movie->hasCdArt());
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
    if (m_info == MovieFilters::Country)
        return (m_hasInfo && movie->countries().contains(m_shortText)) || (!m_hasInfo && movie->countries().isEmpty());
    if (m_info == MovieFilters::Tags)
        return (m_hasInfo && movie->tags().contains(m_shortText)) || (!m_hasInfo && movie->tags().isEmpty());
    if (m_info == MovieFilters::Director)
        return (m_hasInfo && movie->director() == m_shortText) || (!m_hasInfo && movie->director().isEmpty());

    if (m_info == MovieFilters::Path) {
        foreach (const QString &file, movie->files()) {
            if (file.contains(m_shortText, Qt::CaseInsensitive))
                return true;
        }
        return false;
    }

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
