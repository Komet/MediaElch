#pragma once

#include <QNetworkAccessManager>
#include <QObject>

class TvShow;
class TheTvDb;

/// @brief Updates all TvShows, e.g. downloads missing episodes.
class TvShowUpdater : public QObject
{
    Q_OBJECT
public:
    explicit TvShowUpdater(QObject* parent = nullptr);
    static TvShowUpdater* instance(QObject* parent = nullptr);
    void updateShow(TvShow* show, bool force = false);

private:
    TheTvDb* m_tvdb;
    QVector<TvShow*> m_updatedShows;
};
