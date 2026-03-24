#pragma once

#include <QDate>
#include <QString>
#include <QStringList>
#include <QVector>

namespace mediaelch {

namespace scraper {
class ImdbSearchPage
{
public:
    struct SearchResult
    {
        /// \brief Title shown to the user.
        QString title;
        /// \brief Release date of the media item.
        /// \details Date is used for showing the year behind the search result.
        QDate released;
        /// \brief Scraper specific identifier that may be used for scraping an IMDB page.
        /// \details The identifier can be passed to a load job.
        QString identifier;
    };

public:
    /// \brief Parse search results from the IMDB Suggest API JSON response.
    /// \param json The JSON response from v3.sg.media-imdb.com/suggestion/
    /// \param typeFilter Comma-separated list of IMDB title types to include
    ///        (e.g. "feature,tv_movie" for movies, "tvSeries,tvMiniSeries" for TV).
    ///        If empty, all types are included.
    static QVector<SearchResult> parseSuggestResponse(const QString& json, const QStringList& typeFilter = {});

    /// \brief Parse search results from HTML (legacy, will be removed).
    static QVector<SearchResult> parseSearch(const QString& html);
};
} // namespace scraper

} // namespace mediaelch
