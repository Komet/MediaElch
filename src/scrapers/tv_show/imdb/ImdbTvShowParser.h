#pragma once

#include "scrapers/ScraperError.h"
#include "utils/Meta.h"

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <chrono>

#include "data/Locale.h"

class TvShow;

namespace mediaelch {

namespace scraper {

class ImdbTvShowParser : public QObject
{
    Q_OBJECT

public:
    ImdbTvShowParser(TvShow& show, Locale preferredLocale) : m_show{show}, m_preferredLocale{std::move(preferredLocale)}
    {
    }

    ELCH_NODISCARD ScraperError parseInfos(const QString& html);

private:
    TvShow& m_show;
    Locale m_preferredLocale;
};

} // namespace scraper
} // namespace mediaelch
