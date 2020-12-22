#pragma once

#include "globals/Meta.h"

#include <QObject>
#include <QString>
#include <QVector>

namespace mediaelch {
namespace scraper {

class MusicScraper;
class ConcertScraper;
class TvScraper;
class MovieScraper;

} // namespace scraper
} // namespace mediaelch

namespace mediaelch {

class ScraperManager : public QObject
{
    Q_OBJECT

public:
    explicit ScraperManager(QObject* parent = nullptr);
    ~ScraperManager() override = default;

    ELCH_NODISCARD const QVector<mediaelch::scraper::MovieScraper*>& movieScrapers();
    ELCH_NODISCARD const QVector<mediaelch::scraper::TvScraper*>& tvScrapers();
    ELCH_NODISCARD const QVector<mediaelch::scraper::ConcertScraper*>& concertScrapers();
    ELCH_NODISCARD const QVector<mediaelch::scraper::MusicScraper*>& musicScrapers();

    ELCH_NODISCARD mediaelch::scraper::MovieScraper* movieScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::scraper::ConcertScraper* concertScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::scraper::TvScraper* tvScraper(const QString& identifier);

    static ELCH_NODISCARD QVector<mediaelch::scraper::MovieScraper*> constructNativeScrapers(QObject* scraperParent);

private:
    void initMovieScrapers();
    void initTvScrapers();
    void initConcertScrapers();
    void initMusicScrapers();

private:
    QVector<mediaelch::scraper::MovieScraper*> m_movieScrapers;
    QVector<mediaelch::scraper::TvScraper*> m_tvScrapers;
    QVector<mediaelch::scraper::ConcertScraper*> m_concertScrapers;
    QVector<mediaelch::scraper::MusicScraper*> m_musicScrapers;
};

} // namespace mediaelch
