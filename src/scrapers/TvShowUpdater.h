#pragma once

#include <QObject>
#include <QVector>

class TvShow;

namespace mediaelch {
namespace scraper {
class TmdbTv;
}
} // namespace mediaelch

/// \brief   Updates all TvShows, e.g. downloads missing episodes.
/// \details The TVShowUpdater uses TMDB to load missing episodes.
///          The TvShowUpdate requires TvShows to have a valid TMDB ID.
class TvShowUpdater : public QObject
{
    Q_OBJECT
public:
    explicit TvShowUpdater(QObject* parent = nullptr);
    static TvShowUpdater* instance(QObject* parent = nullptr);
    void updateShow(TvShow* show, bool force = false);

private:
    mediaelch::scraper::TmdbTv* m_tmdb;
    QVector<TvShow*> m_updatedShows;
};
