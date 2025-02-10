#include "ui/export/csv_export/CsvExportDialog.h"
#include "ui_CsvExportDialog.h"

#include "globals/Manager.h"
#include "log/Log.h"
#include "utils/Meta.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QRegularExpression>

using namespace mediaelch::exporter;

CsvExportDialog::CsvExportDialog(mediaelch::exporter::CsvExportConfiguration& settings, QWidget* parent) :
    QDialog(parent), ui(new Ui::CsvExportDialog), m_settings{settings}
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    ui->exportProgress->setValue(0);
    ui->lblMessage->clear();

    ui->separator->addItem(tr("Tab"), "\t");
    ui->separator->addItem(tr("Semicolon (;)"), ";");
    ui->separator->addItem(tr("Comma (,)"), ",");
    ui->separator->addItem("Pipe (|)", "|");

    ui->replacement->addItem(tr("Space"), " ");
    ui->replacement->addItem(tr("Semicolon (;)"), ";");
    ui->replacement->addItem(tr("Comma (,)"), ",");
    ui->replacement->addItem(tr("Minus (-)"), "-");
    ui->replacement->addItem("Pipe (|)", "|");

    initializeItems();
    loadSettings();

    connect(ui->comboDetails,
        elchOverload<int>(&QComboBox::activated),
        ui->stackedWidgetDetails,
        &QStackedWidget::setCurrentIndex);
    connect(ui->btnExport, &QPushButton::clicked, this, &CsvExportDialog::onExport);

    // (Un)Check all --------------------------------------

    const auto addCheckAll = [this](QWidget* tab, QListWidget* list) {
        auto* checkbox = new QCheckBox(tr("(Un)Check all"), this);
        bool allChecked = true;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem* item = list->item(i);
            if (item->checkState() != Qt::Checked) {
                allChecked = false;
                break;
            }
        }
        checkbox->setChecked(allChecked);
        tab->layout()->addWidget(checkbox);
        connect(checkbox, &QCheckBox::clicked, this, [this, list](bool checked) { toggleMediaDetails(list, checked); });
    };
    addCheckAll(ui->tabMovieDetails, ui->movieDetailsToExport);
    addCheckAll(ui->tabTvShowDetails, ui->tvShowDetailsToExport);
    addCheckAll(ui->tabEpisodeDetails, ui->tvEpisodeDetailsToExport);
    addCheckAll(ui->tabConcertDetails, ui->concertDetailsToExport);
    addCheckAll(ui->tabArtistDetails, ui->artistDetailsToExport);
    addCheckAll(ui->tabAlbumDetails, ui->albumDetailsToExport);

    // ----------------------------------------------------

    connect(this, &CsvExportDialog::finished, this, &CsvExportDialog::saveSettings);
}

CsvExportDialog::~CsvExportDialog()
{
    delete ui;
}

int CsvExportDialog::exec()
{
    adjustSize();

    return QDialog::exec();
}

void CsvExportDialog::onExport()
{
    using namespace mediaelch;

    m_shouldAbort = false;
    ui->exportProgress->setValue(0);
    ui->btnExport->setEnabled(false);

    // Get separator and replacement characters

    QString separator = "\t";
    QString replacement = " ";

    // Only allow certain characters for now
    // The UI should not allow other values but better be sure.
    if (QRegularExpression("[,;|\t\\s]").match(ui->separator->currentText()).hasMatch()) {
        separator = ui->separator->currentData(Qt::UserRole).toString();
    }
    if (QRegularExpression("[,;|\t\\s-]").match(ui->replacement->currentText()).hasMatch()) {
        replacement = ui->replacement->currentData(Qt::UserRole).toString();
    }

    // Get user's directory where to store the file
    QString location = QFileDialog::getExistingDirectory(this, tr("Export directory"), QDir::homePath());
    if (location.isEmpty()) {
        ui->lblMessage->setErrorMessage(tr("Export aborted. No directory was selected."));
        ui->btnExport->setEnabled(true);
        return;
    }

    QElapsedTimer timer;
    timer.start();

    QDir exportDir(location);

    // Movies ------------------------------------------
    if (!m_shouldAbort && ui->checkMovies->isChecked()) {
        const QVector<Movie*>& movies = Manager::instance()->movieModel()->movies();

        int processedCount = 0;
        ui->exportProgress->setRange(0, qsizetype_to_int(movies.size()));
        ui->lblMessage->setStatusMessage(tr("Export movies..."));

        QFile file(exportFilePath(exportDir, "movies"));
        openFileWithStream(file, [&](QTextStream& stream) {
            // Export with a progress bar (even though it may be so fast that it's not noticeable)
            CsvMovieExport exporter(stream, getFields<CsvMovieExport::Field>(ui->movieDetailsToExport));
            exporter.setSeparator(separator);
            exporter.setReplacement(replacement);
            exporter.exportMovies(movies, [&]() { ui->exportProgress->setValue(++processedCount); });
        });
    }
    // TV shows ----------------------------------------
    if (!m_shouldAbort) {
        const QVector<TvShow*>& tvShows = Manager::instance()->tvShowModel()->tvShows();
        if (ui->checkTvShows->isChecked()) {
            int processedCount = 0;
            ui->exportProgress->setRange(0, qsizetype_to_int(tvShows.size()));
            ui->lblMessage->setStatusMessage(tr("Export TV shows..."));

            QFile showFile(exportFilePath(exportDir, "tv_shows"));
            openFileWithStream(showFile, [&](QTextStream& stream) {
                // Export with a progress bar (even though it may be so fast that it's not noticeable)
                CsvTvShowExport exporter(stream, getFields<CsvTvShowExport::Field>(ui->tvShowDetailsToExport));
                exporter.setSeparator(separator);
                exporter.setReplacement(replacement);
                exporter.exportTvShows(tvShows, [&]() { ui->exportProgress->setValue(++processedCount); });
            });
        }
        if (ui->checkTvEpisodes->isChecked()) {
            int processedCount = 0;
            ui->exportProgress->setRange(0, qsizetype_to_int(tvShows.size()));
            ui->lblMessage->setStatusMessage(tr("Export TV episodes..."));

            QFile episodeFile(exportFilePath(exportDir, "tv_episodes"));
            openFileWithStream(episodeFile, [&](QTextStream& stream) {
                // Export with a progress bar (even though it may be so fast that it's not noticeable)
                CsvTvEpisodeExport exporter(stream, getFields<CsvTvEpisodeExport::Field>(ui->tvEpisodeDetailsToExport));
                exporter.setSeparator(separator);
                exporter.setReplacement(replacement);
                exporter.exportEpisodes(tvShows, [&]() { ui->exportProgress->setValue(++processedCount); });
            });
        }
    }
    // Concerts ----------------------------------------
    if (!m_shouldAbort && ui->checkConcerts->isChecked()) {
        const QVector<Concert*>& concerts = Manager::instance()->concertModel()->concerts();

        int processedCount = 0;
        ui->exportProgress->setRange(0, qsizetype_to_int(concerts.size()));
        ui->lblMessage->setStatusMessage(tr("Export concerts..."));

        QFile episodeFile(exportFilePath(exportDir, "concerts"));
        openFileWithStream(episodeFile, [&](QTextStream& stream) {
            // Export with a progress bar (even though it may be so fast that it's not noticeable)
            CsvConcertExport exporter(stream, getFields<CsvConcertExport::Field>(ui->concertDetailsToExport));
            exporter.setSeparator(separator);
            exporter.setReplacement(replacement);
            exporter.exportConcerts(concerts, [&]() { ui->exportProgress->setValue(++processedCount); });
        });
    }
    // Music -------------------------------------------
    if (!m_shouldAbort) {
        const QVector<Artist*>& artists = Manager::instance()->musicModel()->artists();
        // Artists ----------------------------------------
        if (ui->checkMusicArtists->isChecked()) {
            int processedCount = 0;
            ui->exportProgress->setRange(0, qsizetype_to_int(artists.size()));
            ui->lblMessage->setStatusMessage(tr("Export artists..."));

            QFile episodeFile(exportFilePath(exportDir, "artists"));
            openFileWithStream(episodeFile, [&](QTextStream& stream) {
                // Export with a progress bar (even though it may be so fast that it's not noticeable)
                CsvArtistExport exporter(stream, getFields<CsvArtistExport::Field>(ui->artistDetailsToExport));
                exporter.setSeparator(separator);
                exporter.setReplacement(replacement);
                exporter.exportArtists(artists, [&]() { ui->exportProgress->setValue(++processedCount); });
            });
        }
        // Albums ----------------------------------------
        if (ui->checkMusicAlbums->isChecked()) {
            int processedCount = 0;
            ui->exportProgress->setRange(0, qsizetype_to_int(artists.size()));
            ui->lblMessage->setStatusMessage(tr("Export albums..."));

            QFile episodeFile(exportFilePath(exportDir, "albums"));
            openFileWithStream(episodeFile, [&](QTextStream& stream) {
                // Export with a progress bar (even though it may be so fast that it's not noticeable)
                CsvAlbumExport exporter(stream, getFields<CsvAlbumExport::Field>(ui->albumDetailsToExport));
                exporter.setSeparator(separator);
                exporter.setReplacement(replacement);
                exporter.exportAlbumsOfArtists(artists, [&]() { ui->exportProgress->setValue(++processedCount); });
            });
        }
    }
    // ------------------------------------------
    if (!m_shouldAbort) {
        QString secondsElapsed = QString::number(static_cast<double>(timer.elapsed()) / 1000.0);
        ui->lblMessage->setSuccessMessage(tr("Export completed in %1 seconds.").arg(secondsElapsed));
        qCInfo(generic) << "[CsvExport] Finished successfully in" << secondsElapsed << "seconds";
    }

    // Set progress bar to "done" state
    ui->exportProgress->setRange(0, 1);
    ui->exportProgress->setValue(1);
    ui->btnExport->setEnabled(true);
}

void CsvExportDialog::saveSettings()
{
    m_settings.setCsvExportSeparator(ui->separator->currentData().toString());
    m_settings.setCsvExportReplacement(ui->replacement->currentData().toString());
    {
        QVector<CsvExportType> mediaToExport;
        if (ui->checkMovies->isChecked()) {
            mediaToExport << CsvExportType::Movies;
        }
        if (ui->checkConcerts->isChecked()) {
            mediaToExport << CsvExportType::Concerts;
        }
        if (ui->checkTvShows->isChecked()) {
            mediaToExport << CsvExportType::TvShows;
        }
        if (ui->checkTvEpisodes->isChecked()) {
            mediaToExport << CsvExportType::TvEpisodes;
        }
        if (ui->checkMusicArtists->isChecked()) {
            mediaToExport << CsvExportType::MusicArtists;
        }
        if (ui->checkMusicAlbums->isChecked()) {
            mediaToExport << CsvExportType::MusicAlbums;
        }
        m_settings.setCsvExportTypes(mediaToExport);
    }

    m_settings.setCsvExportMovieFields(getFieldsAsStrings<mediaelch::CsvMovieExport>(ui->movieDetailsToExport));
    m_settings.setCsvExportConcertFields(getFieldsAsStrings<mediaelch::CsvConcertExport>(ui->concertDetailsToExport));
    m_settings.setCsvExportTvShowFields(getFieldsAsStrings<mediaelch::CsvTvShowExport>(ui->tvShowDetailsToExport));
    m_settings.setCsvExportTvEpisodeFields(
        getFieldsAsStrings<mediaelch::CsvTvEpisodeExport>(ui->tvEpisodeDetailsToExport));
    m_settings.setCsvExportMusicArtistFields(getFieldsAsStrings<mediaelch::CsvArtistExport>(ui->artistDetailsToExport));
    m_settings.setCsvExportMusicAlbumFields(getFieldsAsStrings<mediaelch::CsvAlbumExport>(ui->albumDetailsToExport));
}

void CsvExportDialog::initializeItems()
{
    using namespace mediaelch;

    {
        using Field = CsvMovieExport::Field;
        ui->movieDetailsToExport->clear();

        const QStringList fields = m_settings.csvExportMovieFields();
        const auto addField = [this, &fields](Field field, const QString& name) {
            const bool isChecked = fields.isEmpty() || fields.contains(CsvMovieExport::fieldToString(field));
            auto* item = new QListWidgetItem(name, ui->movieDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        };

        // TODO: Compile time / runtime check for completness
        addField(Field::Type, tr("Type"));
        addField(Field::ImdbId, tr("IMDb ID"));
        addField(Field::TmdbId, tr("TMDB ID"));
        addField(Field::WikidataId, tr("Wikidata ID"));
        addField(Field::Title, tr("Title"));
        addField(Field::OriginalTitle, tr("Original Title"));
        addField(Field::SortTitle, tr("Sort Title"));
        addField(Field::Overview, tr("Overview"));
        addField(Field::Outline, tr("Outline"));
        addField(Field::Ratings, tr("Rating"));
        addField(Field::UserRating, tr("User Rating"));
        addField(Field::IsImdbTop250, tr("IMDb Top 250"));
        addField(Field::ReleaseDate, tr("Release Date"));
        addField(Field::Tagline, tr("Tagline"));
        addField(Field::Runtime, tr("Runtime in minutes"));
        addField(Field::Certification, tr("Certification"));
        addField(Field::Writers, tr("Writers"));
        addField(Field::Directors, tr("Director"));
        addField(Field::Genres, tr("Genres"));
        addField(Field::Countries, tr("Countries"));
        addField(Field::Studios, tr("Studios"));
        addField(Field::Tags, tr("Tags"));
        addField(Field::Trailer, tr("Trailers"));
        addField(Field::Actors, tr("Actors"));
        addField(Field::PlayCount, tr("Playcount"));
        addField(Field::LastPlayed, tr("Last played"));
        addField(Field::MovieSet, tr("Movie Set"));
        addField(Field::TvShowLinks, tr("TV Show Links"));
        addField(Field::Directory, tr("Directory"));
        addField(Field::Filenames, tr("Filename(s)"));
        addField(Field::LastModified, tr("Last Modified Date"));
        addField(Field::StreamDetails_Video_DurationInSeconds, tr("Streamdetails - Duration (in seconds)"));
        addField(Field::StreamDetails_Video_Aspect, tr("Streamdetails - Video Aspect Ratio"));
        addField(Field::StreamDetails_Video_Width, tr("Streamdetails - Video Width"));
        addField(Field::StreamDetails_Video_Height, tr("Streamdetails - Video Height"));
        addField(Field::StreamDetails_Video_Codec, tr("Streamdetails - Video Codec"));
        addField(Field::StreamDetails_Audio_Language, tr("Streamdetails - Audio Language(s)"));
        addField(Field::StreamDetails_Audio_Codec, tr("Streamdetails - Audio Codec(s)"));
        addField(Field::StreamDetails_Audio_Channels, tr("Streamdetails - Audio Channel(s)"));
        addField(Field::StreamDetails_Subtitle_Language, tr("Streamdetails - Subtitle Language(s)"));
    }
    {
        using Field = CsvTvShowExport::Field;

        ui->tvShowDetailsToExport->clear();

        const QStringList fields = m_settings.csvExportTvShowFields();
        const auto addField = [this, &fields](Field field, const QString& name) {
            const bool isChecked = fields.isEmpty() || fields.contains(CsvTvShowExport::fieldToString(field));
            auto* item = new QListWidgetItem(name, ui->tvShowDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        };

        addField(Field::Type, tr("Type"));
        addField(Field::ShowTmdbId, tr("TMDB ID"));
        addField(Field::ShowImdbId, tr("IMDb ID"));
        addField(Field::ShowTvDbId, tr("TheTvDb ID"));
        addField(Field::ShowTvMazeId, tr("TVmaze ID"));
        addField(Field::ShowTitle, tr("Title"));
        addField(Field::ShowSortTitle, tr("Sort Title"));
        addField(Field::ShowOriginalTitle, tr("Original Title"));
        addField(Field::ShowFirstAired, tr("First Aired"));
        addField(Field::ShowNetwork, tr("network"));
        addField(Field::ShowCertification, tr("Certification"));
        addField(Field::ShowGenres, tr("Genres"));
        addField(Field::ShowTags, tr("Tags"));
        addField(Field::ShowRuntime, tr("Runtime"));
        addField(Field::ShowRatings, tr("Ratings"));
        addField(Field::ShowIsImdbTop250, tr("IMDb Top 250"));
        addField(Field::ShowUserRating, tr("User Rating"));
        addField(Field::ShowActors, tr("Actors"));
        addField(Field::ShowOverview, tr("Overview"));
        addField(Field::ShowDirectory, tr("Directory"));
    }
    {
        using Field = CsvTvEpisodeExport::Field;

        ui->tvEpisodeDetailsToExport->clear();

        const QStringList fields = m_settings.csvExportTvEpisodeFields();
        const auto addField = [this, &fields](Field field, const QString& name) {
            const bool isChecked = fields.isEmpty() || fields.contains(CsvTvEpisodeExport::fieldToString(field));
            auto* item = new QListWidgetItem(name, ui->tvEpisodeDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        };

        addField(Field::Type, tr("Type"));
        addField(Field::ShowImdbId, tr("TV Show - IMDb ID"));
        addField(Field::ShowTmdbId, tr("TV Show - TMDB ID"));
        addField(Field::ShowTvDbId, tr("TV Show - TheTvDb ID"));
        addField(Field::ShowTvMazeId, tr("TV Show - TVmaze ID"));
        addField(Field::ShowTitle, tr("TV Show - Title"));
        addField(Field::EpisodeSeason, tr("Episode - Season"));
        addField(Field::EpisodeNumber, tr("Episode - Number"));
        addField(Field::EpisodeImdbId, tr("Episode - IMDb ID"));
        addField(Field::EpisodeTmdbId, tr("Episode - TMDB ID"));
        addField(Field::EpisodeTvDbId, tr("Episode - TheTvDb ID"));
        addField(Field::EpisodeTvMazeId, tr("Episode - TVmaze ID"));
        addField(Field::EpisodeFirstAired, tr("Episode - First Aired"));
        addField(Field::EpisodeTitle, tr("Episode - Title"));
        addField(Field::EpisodeOverview, tr("Episode - Overview"));
        addField(Field::EpisodeUserRating, tr("Episode - User Rating"));
        addField(Field::EpisodeDirectors, tr("Episode - Directors"));
        addField(Field::EpisodeWriters, tr("Episode - Writers"));
        addField(Field::EpisodeActors, tr("Episode - Actors"));
        addField(Field::EpisodeDirectory, tr("Episode - Directory"));
        addField(Field::EpisodeFilenames, tr("Episode - Filename(s)"));
        addField(Field::EpisodeStreamDetails_Video_DurationInSeconds, tr("Streamdetails - Duration (in seconds)"));
        addField(Field::EpisodeStreamDetails_Video_Aspect, tr("Streamdetails - Video Aspect"));
        addField(Field::EpisodeStreamDetails_Video_Width, tr("Streamdetails - Video Width"));
        addField(Field::EpisodeStreamDetails_Video_Height, tr("Streamdetails - Video Height"));
        addField(Field::EpisodeStreamDetails_Video_Codec, tr("Streamdetails - Video Codec"));
        addField(Field::EpisodeStreamDetails_Audio_Language, tr("Streamdetails - Audio Language(s)"));
        addField(Field::EpisodeStreamDetails_Audio_Codec, tr("Streamdetails - Audio Codec(s)"));
        addField(Field::EpisodeStreamDetails_Audio_Channels, tr("Streamdetails - Audio Channel(s)"));
        addField(Field::EpisodeStreamDetails_Subtitle_Language, tr("Streamdetails - Subtitle Language(s)"));
    }
    {
        using Field = CsvConcertExport::Field;
        ui->concertDetailsToExport->clear();

        const QStringList fields = m_settings.csvExportConcertFields();
        const auto addField = [this, &fields](Field field, const QString& name) {
            const bool isChecked = fields.isEmpty() || fields.contains(CsvConcertExport::fieldToString(field));
            auto* item = new QListWidgetItem(name, ui->concertDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        };

        addField(Field::Type, tr("Type"));
        addField(Field::Title, tr("Title"));
        addField(Field::TmdbId, tr("TMDB ID"));
        addField(Field::ImdbId, tr("IMDb ID"));
        addField(Field::OriginalTitle, tr("Original Title"));
        addField(Field::Artist, tr("Artist"));
        addField(Field::Album, tr("Album"));
        addField(Field::Overview, tr("Overview"));
        addField(Field::Ratings, tr("Ratings"));
        addField(Field::UserRating, tr("User Rating"));
        addField(Field::ReleaseDate, tr("Release Date"));
        addField(Field::Tagline, tr("Tagline"));
        addField(Field::Runtime, tr("Runtime"));
        addField(Field::Certification, tr("Certification"));
        addField(Field::Genres, tr("Genres"));
        addField(Field::Tags, tr("Tags"));
        addField(Field::TrailerUrl, tr("Trailer URL"));
        addField(Field::Playcount, tr("Playcount"));
        addField(Field::LastPlayed, tr("Last Played"));
        addField(Field::LastModified, tr("Last Modified Date"));
        addField(Field::Directory, tr("Directory"));
        addField(Field::Filenames, tr("Filename(s)"));
        addField(Field::StreamDetails_Video_DurationInSeconds, tr("Streamdetails - Duration (in seconds)"));
        addField(Field::StreamDetails_Video_Aspect, tr("Streamdetails - Video Aspect"));
        addField(Field::StreamDetails_Video_Width, tr("Streamdetails - Video Width"));
        addField(Field::StreamDetails_Video_Height, tr("Streamdetails - Video Height"));
        addField(Field::StreamDetails_Video_Codec, tr("Streamdetails - Video Codec"));
        addField(Field::StreamDetails_Audio_Language, tr("Streamdetails - Audio Language(s)"));
        addField(Field::StreamDetails_Audio_Codec, tr("Streamdetails - Audio Codec(s)"));
        addField(Field::StreamDetails_Audio_Channels, tr("Streamdetails - Audio Channel(s)"));
        addField(Field::StreamDetails_Subtitle_Language, tr("Streamdetails - Subtitle Language(s)"));
    }
    {
        using Field = CsvArtistExport::Field;
        ui->artistDetailsToExport->clear();

        const QStringList fields = m_settings.csvExportMusicArtistFields();
        const auto addField = [this, &fields](Field field, const QString& name) {
            const bool isChecked = fields.isEmpty() || fields.contains(CsvArtistExport::fieldToString(field));
            auto* item = new QListWidgetItem(name, ui->artistDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        };

        addField(Field::Type, tr("Type"));
        addField(Field::ArtistName, tr("Name"));
        addField(Field::ArtistGenres, tr("Genres"));
        addField(Field::ArtistStyles, tr("Styles"));
        addField(Field::ArtistMoods, tr("Moods"));
        addField(Field::ArtistYearsActive, tr("Years Active"));
        addField(Field::ArtistFormed, tr("Formed"));
        addField(Field::ArtistBiography, tr("Biography"));
        addField(Field::ArtistBorn, tr("Born"));
        addField(Field::ArtistDied, tr("Died"));
        addField(Field::ArtistDisbanded, tr("Disbanded"));
        addField(Field::ArtistMusicBrainzId, tr("MusicBrainz ID"));
        addField(Field::ArtistAllMusicId, tr("AllMusic ID"));
        addField(Field::ArtistDirectory, tr("Directory"));
    }
    {
        using Field = CsvAlbumExport::Field;
        ui->albumDetailsToExport->clear();

        const QStringList fields = m_settings.csvExportMusicAlbumFields();
        const auto addField = [this, &fields](Field field, const QString& name) {
            const bool isChecked = fields.isEmpty() || fields.contains(CsvAlbumExport::fieldToString(field));
            auto* item = new QListWidgetItem(name, ui->albumDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        };

        addField(Field::Type, tr("Type"));
        addField(Field::ArtistName, tr("Artist - Name"));
        addField(Field::AlbumTitle, tr("Album - Title"));
        addField(Field::AlbumArtistName, tr("Album - Artist Name"));
        addField(Field::AlbumGenres, tr("Album - Genres"));
        addField(Field::AlbumStyles, tr("Album - Styles"));
        addField(Field::AlbumMoods, tr("Album - Moods"));
        addField(Field::AlbumReview, tr("Album - Review"));
        addField(Field::AlbumReleaseDate, tr("Album - Release Date"));
        addField(Field::AlbumLabel, tr("Album - Label"));
        addField(Field::AlbumRating, tr("Album - Rating"));
        addField(Field::AlbumYear, tr("Album - Year"));
        addField(Field::AlbumMusicBrainzId, tr("Album - MusicBrainz ID"));
        addField(Field::AlbumMusicBrainzReleaseGroupId, tr("Album - MusicBrainz ReleaseGroup ID"));
        addField(Field::AlbumAllMusicId, tr("Album - AllMusic ID"));
        addField(Field::AlbumDirectory, tr("Album - Directory"));
    }
}

void CsvExportDialog::loadSettings()
{
    {
        int index = ui->replacement->findData(m_settings.csvExportReplacement());
        index = index < 0 ? 0 : index;
        ui->replacement->setCurrentIndex(index);
    }
    {
        int index = ui->separator->findData(m_settings.csvExportSeparator());
        index = index < 0 ? 0 : index;
        ui->separator->setCurrentIndex(index);
    }
    const auto& mediaToExport = m_settings.csvExportTypes();
    if (!mediaToExport.isEmpty()) {
        ui->checkMovies->setChecked(mediaToExport.contains(CsvExportType::Movies));
        ui->checkConcerts->setChecked(mediaToExport.contains(CsvExportType::Concerts));
        ui->checkTvShows->setChecked(mediaToExport.contains(CsvExportType::TvShows));
        ui->checkTvEpisodes->setChecked(mediaToExport.contains(CsvExportType::TvEpisodes));
        ui->checkMusicArtists->setChecked(mediaToExport.contains(CsvExportType::MusicArtists));
        ui->checkMusicAlbums->setChecked(mediaToExport.contains(CsvExportType::MusicAlbums));
    }
}

bool CsvExportDialog::openFileOrPrintError(QFile& file)
{
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        ui->lblMessage->setErrorMessage(tr("Export failed. File could not be opened for writing."));
        qCInfo(generic) << "[CsvExport] Failed: Could not open file";
        return false;
    }
    return true;
}

bool CsvExportDialog::checkTextStreamStatus(QTextStream& stream)
{
    if (stream.status() != QTextStream::Ok) {
        ui->lblMessage->setErrorMessage(tr("Export failed. Could not write to CSV file."));
        qCInfo(generic) << "[CsvExport] Failed";
        return false;
    }
    return true;
}

QString CsvExportDialog::exportFilePath(const QDir& dir, const QString& filename) const
{
    return dir.path() + "/" + defaultCsvFileName(filename);
}

QString CsvExportDialog::defaultCsvFileName(const QString& type) const
{
    return QStringLiteral("MediaElch_%1_%2.csv") //
        .arg(type, QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
}

void CsvExportDialog::toggleMediaDetails(QListWidget* widget, bool isChecked)
{
    for (int i = 0; i < widget->count(); ++i) {
        QListWidgetItem* item = widget->item(i);
        item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
    }
}
