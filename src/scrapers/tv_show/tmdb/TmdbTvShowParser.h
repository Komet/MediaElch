#pragma once

#include "data/Locale.h"

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <chrono>

class TvShow;
struct ScraperError;

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbTvShowParser : public QObject
{
    Q_OBJECT

public:
    TmdbTvShowParser(const TmdbApi& api, TvShow& show, ScraperError& error, QObject* parent = nullptr) :
        QObject(parent), m_api{api}, m_show{show}, m_error{error}
    {
    }

    /// \brief Parse the given JSON document and assign the details to the parser's show.
    /// \param json JSON document from TMDb
    /// \param locale Locale used to identify the correct certification for the given country.
    void parseInfos(const QJsonDocument& json, const Locale& locale);

private:
    const TmdbApi& m_api;
    TvShow& m_show;
    ScraperError& m_error;
};

} // namespace scraper
} // namespace mediaelch
