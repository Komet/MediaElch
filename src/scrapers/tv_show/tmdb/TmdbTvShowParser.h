#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperError.h"

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <chrono>

class TvShow;

namespace mediaelch {

namespace scraper {

class TmdbApi;

class TmdbTvShowParser : public QObject
{
    Q_OBJECT

public:
    TmdbTvShowParser(const TmdbApi& api, TvShow& show, QObject* parent = nullptr) :
        QObject(parent), m_api{api}, m_show{show}
    {
    }

    /// \brief Parse the given JSON document and assign the details to the parser's show.
    /// \param json JSON document from TMDB
    /// \param locale Locale used to identify the correct certification for the given country.
    void parseInfos(const QJsonDocument& json, const Locale& locale);

private:
    const TmdbApi& m_api;
    TvShow& m_show;
};

} // namespace scraper
} // namespace mediaelch
