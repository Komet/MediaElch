#pragma once

#include "database/Database.h"
#include "file_search/ConcertFileSearcher.h"
#include "file_search/MusicFileSearcher.h"
#include "file_search/TvShowFileSearcher.h"
#include "file_search/movie/MovieFileSearcher.h"
#include "media_center/MediaCenterInterface.h"
#include "model/ConcertModel.h"
#include "model/MovieModel.h"
#include "model/TvShowModel.h"
#include "model/TvShowProxyModel.h"
#include "model/music/MusicModel.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/image/ImageProvider.h"
#include "scrapers/trailer/TrailerProvider.h"
#include "settings/KodiSettings.h"
#include "settings/Settings.h"
#include "ui/main/FileScannerDialog.h"
#include "ui/main/MyIconFont.h"
#include "ui/music/MusicFilesWidget.h"
#include "ui/scrapers/ScraperManager.h"
#include "ui/tv_show/TvShowFilesWidget.h"

#include <QString>
#include <QVector>

namespace mediaelch {
namespace exporter {
class CsvExportModule;
}
} // namespace mediaelch

class MediaCenterInterface;

/// \brief Central class for various instances, e.g. scrapers and database.
class Manager : public QObject
{
    Q_OBJECT

public:
    explicit Manager(QObject* parent = nullptr);
    ~Manager() override = default;

    static Manager* instance();
    ELCH_NODISCARD mediaelch::ScraperManager& scrapers();
    ELCH_NODISCARD QVector<mediaelch::scraper::TrailerProvider*> trailerProviders();
    ELCH_NODISCARD MediaCenterInterface* mediaCenterInterface();
    ELCH_NODISCARD MediaCenterInterface* mediaCenterInterfaceTvShow();
    ELCH_NODISCARD MediaCenterInterface* mediaCenterInterfaceConcert();
    ELCH_NODISCARD mediaelch::MovieFileSearcher* movieFileSearcher();
    ELCH_NODISCARD TvShowFileSearcher* tvShowFileSearcher();
    ELCH_NODISCARD ConcertFileSearcher* concertFileSearcher();
    ELCH_NODISCARD MusicFileSearcher* musicFileSearcher();
    ELCH_NODISCARD Database* database();
    ELCH_NODISCARD MovieModel* movieModel();
    ELCH_NODISCARD TvShowModel* tvShowModel();
    ELCH_NODISCARD ConcertModel* concertModel();
    ELCH_NODISCARD MusicModel* musicModel();
    ELCH_NODISCARD FileScannerDialog* fileScannerDialog();
    ELCH_NODISCARD mediaelch::scraper::FanartTv* fanartTv();
    ELCH_NODISCARD TvShowFilesWidget* tvShowFilesWidget();
    ELCH_NODISCARD MusicFilesWidget* musicFilesWidget();
    ELCH_NODISCARD MyIconFont* iconFont();
    void setTvShowFilesWidget(TvShowFilesWidget* widget);
    void setMusicFilesWidget(MusicFilesWidget* widget);
    void setFileScannerDialog(FileScannerDialog* dialog);

    ELCH_NODISCARD mediaelch::KodiSettings* kodiSettings();

    ELCH_NODISCARD mediaelch::exporter::CsvExportModule& csvExportModule();

private:
    mediaelch::KodiSettings* m_kodiSettings;
    QVector<MediaCenterInterface*> m_mediaCenters;
    QVector<MediaCenterInterface*> m_mediaCentersTvShow;
    QVector<MediaCenterInterface*> m_mediaCentersConcert;
    QVector<mediaelch::scraper::TrailerProvider*> m_trailerProviders;

    Settings* m_settings = nullptr;
    mediaelch::ScraperManager* m_scraperManager = nullptr;
    mediaelch::MovieFileSearcher* m_movieFileSearcher = nullptr;
    TvShowFileSearcher* m_tvShowFileSearcher = nullptr;
    ConcertFileSearcher* m_concertFileSearcher = nullptr;
    MovieModel* m_movieModel = nullptr;
    TvShowModel* m_tvShowModel = nullptr;
    ConcertModel* m_concertModel = nullptr;
    MusicModel* m_musicModel = nullptr;
    Database* m_database = nullptr;
    TvShowFilesWidget* m_tvShowFilesWidget = nullptr;
    MusicFilesWidget* m_musicFilesWidget = nullptr;
    FileScannerDialog* m_fileScannerDialog = nullptr;
    MusicFileSearcher* m_musicFileSearcher = nullptr;
    MyIconFont* m_iconFont = nullptr;

    mediaelch::exporter::CsvExportModule* m_csvExportModule;
};
