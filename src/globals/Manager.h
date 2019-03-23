#pragma once

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
#include "imageProviders/FanartTv.h"
#include "main/FileScannerDialog.h"
#include "main/MyIconFont.h"
#include "music/MusicFileSearcher.h"
#include "music/MusicFilesWidget.h"
#include "music/MusicModel.h"
#include "scrapers/music/TvTunes.h"
#include "settings/Settings.h"
#include "trailerProviders/TrailerProvider.h"
#include "tvShows/TvShowModel.h"
#include "tvShows/TvShowProxyModel.h"
#include "ui/tv_show/TvShowFilesWidget.h"

#include <QString>
#include <QVector>

class MediaCenterInterface;

/**
 * @brief The Manager class
 * This class handles the various interfaces
 */
class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject* parent = nullptr);
    ~Manager() override;

    static Manager* instance();
    QVector<MediaCenterInterface*> mediaCenters();
    QVector<MovieScraperInterface*> movieScrapers();
    MovieScraperInterface* scraper(const QString& identifier);
    QVector<TvScraperInterface*> tvScrapers();
    QVector<ConcertScraperInterface*> concertScrapers();
    QVector<MusicScraperInterface*> musicScrapers();
    QVector<ImageProviderInterface*> imageProviders();
    QVector<ImageProviderInterface*> imageProviders(ImageType type);
    QVector<TrailerProvider*> trailerProviders();
    MediaCenterInterface* mediaCenterInterface();
    MediaCenterInterface* mediaCenterInterfaceTvShow();
    MediaCenterInterface* mediaCenterInterfaceConcert();
    MovieFileSearcher* movieFileSearcher();
    TvShowFileSearcher* tvShowFileSearcher();
    ConcertFileSearcher* concertFileSearcher();
    MusicFileSearcher* musicFileSearcher();
    Database* database();
    MovieModel* movieModel();
    TvShowModel* tvShowModel();
    ConcertModel* concertModel();
    MusicModel* musicModel();
    FileScannerDialog* fileScannerDialog();
    FanartTv* fanartTv();
    TvShowFilesWidget* tvShowFilesWidget();
    MusicFilesWidget* musicFilesWidget();
    TvTunes* tvTunes();
    MyIconFont* iconFont();
    void setTvShowFilesWidget(TvShowFilesWidget* widget);
    void setMusicFilesWidget(MusicFilesWidget* widget);
    void setFileScannerDialog(FileScannerDialog* dialog);
    static QVector<MovieScraperInterface*> constructNativeScrapers(QObject* parent);

private:
    QVector<MediaCenterInterface*> m_mediaCenters;
    QVector<MediaCenterInterface*> m_mediaCentersTvShow;
    QVector<MediaCenterInterface*> m_mediaCentersConcert;
    QVector<MovieScraperInterface*> m_scrapers;
    QVector<TvScraperInterface*> m_tvScrapers;
    QVector<ConcertScraperInterface*> m_concertScrapers;
    QVector<MusicScraperInterface*> m_musicScrapers;
    QVector<ImageProviderInterface*> m_imageProviders;
    QVector<TrailerProvider*> m_trailerProviders;
    MovieFileSearcher* m_movieFileSearcher;
    TvShowFileSearcher* m_tvShowFileSearcher;
    ConcertFileSearcher* m_concertFileSearcher;
    MovieModel* m_movieModel;
    TvShowModel* m_tvShowModel;
    ConcertModel* m_concertModel;
    MusicModel* m_musicModel;
    Settings* m_settings;
    Database* m_database;
    TvShowFilesWidget* m_tvShowFilesWidget;
    MusicFilesWidget* m_musicFilesWidget;
    FileScannerDialog* m_fileScannerDialog;
    TvTunes* m_tvTunes;
    MusicFileSearcher* m_musicFileSearcher;
    MyIconFont* m_iconFont;
};
