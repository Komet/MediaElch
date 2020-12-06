#pragma once

#include <QObject>
#include <QVector>

class TvShow;

namespace mediaelch {
namespace scraper {
class TheTvDb;
}
} // namespace mediaelch

/// \brief   Updates all TvShows, e.g. downloads missing episodes.
/// \details The TVShowUpdater uses TheTvDb to load missing episodes.
///          The TvShowUpdate requires TvShows to have a valid TvDbId.
class TvShowUpdater : public QObject
{
    Q_OBJECT
public:
    explicit TvShowUpdater(QObject* parent = nullptr);
    static TvShowUpdater* instance(QObject* parent = nullptr);
    void updateShow(TvShow* show, bool force = false);

private:
    mediaelch::scraper::TheTvDb* m_tvdb;
    QVector<TvShow*> m_updatedShows;
};
