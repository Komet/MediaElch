#ifndef MANAGER_H
#define MANAGER_H

#include "data/ConcertFileSearcher.h"
#include "data/ConcertModel.h"
#include "data/ConcertScraperInterface.h"
#include "data/Database.h"
#include "data/ImageProviderInterface.h"
#include "data/MediaCenterInterface.h"
#include "data/MovieFileSearcher.h"
#include "data/MovieModel.h"
#include "data/MovieScraperInterface.h"
#include "data/MusicScraperInterface.h"
#include "data/TvScraperInterface.h"
#include "data/TvShowFileSearcher.h"
#include "data/TvShowModel.h"
#include "data/TvShowProxyModel.h"
#include "imageProviders/FanartTv.h"
#include "main/FileScannerDialog.h"
#include "main/MyIconFont.h"
#include "music/MusicFileSearcher.h"
#include "music/MusicFilesWidget.h"
#include "music/MusicModel.h"
#include "scrapers/TvTunes.h"
#include "settings/Settings.h"
#include "trailerProviders/TrailerProvider.h"
#include "tvShows/TvShowFilesWidget.h"

#include <QList>
#include <QString>

class MediaCenterInterface;

/**
 * @brief The Manager class
 * This class handles the various interfaces
 */
class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = nullptr);
    ~Manager() override;

    static Manager *instance();
    QList<MediaCenterInterface *> mediaCenters();
    QList<MovieScraperInterface *> scrapers();
    MovieScraperInterface *scraper(const QString &identifier);
    QList<TvScraperInterface *> tvScrapers();
    QList<ConcertScraperInterface *> concertScrapers();
    QList<MusicScraperInterface *> musicScrapers();
    QList<ImageProviderInterface *> imageProviders();
    QList<ImageProviderInterface *> imageProviders(ImageType type);
    QList<TrailerProvider *> trailerProviders();
    MediaCenterInterface *mediaCenterInterface();
    MediaCenterInterface *mediaCenterInterfaceTvShow();
    MediaCenterInterface *mediaCenterInterfaceConcert();
    MovieFileSearcher *movieFileSearcher();
    TvShowFileSearcher *tvShowFileSearcher();
    ConcertFileSearcher *concertFileSearcher();
    MusicFileSearcher *musicFileSearcher();
    Database *database();
    MovieModel *movieModel();
    TvShowModel *tvShowModel();
    TvShowProxyModel *tvShowProxyModel();
    ConcertModel *concertModel();
    MusicModel *musicModel();
    FileScannerDialog *fileScannerDialog();
    FanartTv *fanartTv();
    TvShowFilesWidget *tvShowFilesWidget();
    MusicFilesWidget *musicFilesWidget();
    TvTunes *tvTunes();
    MyIconFont *iconFont();
    void setTvShowFilesWidget(TvShowFilesWidget *widget);
    void setMusicFilesWidget(MusicFilesWidget *widget);
    void setFileScannerDialog(FileScannerDialog *dialog);
    static QList<MovieScraperInterface *> constructNativeScrapers(QObject *parent);

private:
    QList<MediaCenterInterface *> m_mediaCenters;
    QList<MediaCenterInterface *> m_mediaCentersTvShow;
    QList<MediaCenterInterface *> m_mediaCentersConcert;
    QList<MovieScraperInterface *> m_scrapers;
    QList<TvScraperInterface *> m_tvScrapers;
    QList<ConcertScraperInterface *> m_concertScrapers;
    QList<MusicScraperInterface *> m_musicScrapers;
    QList<ImageProviderInterface *> m_imageProviders;
    QList<TrailerProvider *> m_trailerProviders;
    MovieFileSearcher *m_movieFileSearcher;
    TvShowFileSearcher *m_tvShowFileSearcher;
    ConcertFileSearcher *m_concertFileSearcher;
    MovieModel *m_movieModel;
    TvShowModel *m_tvShowModel;
    TvShowProxyModel *m_tvShowProxyModel;
    ConcertModel *m_concertModel;
    MusicModel *m_musicModel;
    Settings *m_settings;
    Database *m_database;
    TvShowFilesWidget *m_tvShowFilesWidget;
    MusicFilesWidget *m_musicFilesWidget;
    FileScannerDialog *m_fileScannerDialog;
    TvTunes *m_tvTunes;
    MusicFileSearcher *m_musicFileSearcher;
    MyIconFont *m_iconFont;
};

#endif // MANAGER_H
