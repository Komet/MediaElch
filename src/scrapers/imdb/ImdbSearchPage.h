#pragma once

#include <QDate>
#include <QString>
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
    static QVector<SearchResult> parseSearch(const QString& html);
};
} // namespace scraper

} // namespace mediaelch
