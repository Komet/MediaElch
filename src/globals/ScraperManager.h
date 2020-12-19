#pragma once

#include "globals/Meta.h"

#include <QObject>
#include <QString>
#include <QVector>

class ConcertScraperInterface;
class MusicScraperInterface;

namespace mediaelch {
namespace scraper {
class TvScraper;
class MovieScraperInterface;
} // namespace scraper
} // namespace mediaelch

namespace mediaelch {

class ScraperManager : public QObject
{
    Q_OBJECT

public:
    explicit ScraperManager(QObject* parent = nullptr);
    ~ScraperManager() override = default;

    ELCH_NODISCARD const QVector<mediaelch::scraper::MovieScraperInterface*>& movieScrapers();
    ELCH_NODISCARD const QVector<mediaelch::scraper::TvScraper*>& tvScrapers();
    ELCH_NODISCARD const QVector<ConcertScraperInterface*>& concertScrapers();
    ELCH_NODISCARD const QVector<MusicScraperInterface*>& musicScrapers();

    ELCH_NODISCARD mediaelch::scraper::MovieScraperInterface* movieScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::scraper::TvScraper* tvScraper(const QString& identifier);

    static ELCH_NODISCARD QVector<mediaelch::scraper::MovieScraperInterface*> constructNativeScrapers(
        QObject* scraperParent);

private:
    void initMovieScrapers();
    void initTvScrapers();
    void initConcertScrapers();
    void initMusicScrapers();

private:
    QVector<mediaelch::scraper::MovieScraperInterface*> m_movieScrapers;
    QVector<mediaelch::scraper::TvScraper*> m_tvScrapers;
    QVector<ConcertScraperInterface*> m_concertScrapers;
    QVector<MusicScraperInterface*> m_musicScrapers;
};

} // namespace mediaelch
