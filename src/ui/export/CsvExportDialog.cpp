#include "ui/export/CsvExportDialog.h"
#include "ui_CsvExportDialog.h"

#include "globals/Manager.h"

#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QRegularExpression>

CsvExportDialog::CsvExportDialog(QWidget* parent) : QDialog(parent), ui(new Ui::CsvExportDialog)
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

    connect(ui->btnExport, &QPushButton::clicked, this, &CsvExportDialog::onExport);
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

    QDir exportDir(location);

    // Movies ------------------------------------------
    {
        const QVector<Movie*>& movies = Manager::instance()->movieModel()->movies();

        // Export with a progress bar (even though it may be so fast that it's not noticable)
        CsvMovieExport csvExporter(getFields<CsvMovieExport::MovieField>(ui->movieDetailsToExport));
        csvExporter.setSeparator(separator);
        csvExporter.setReplacement(replacement);

        int processedCount = 0;
        ui->exportProgress->setRange(0, movies.size());

        QString csv = csvExporter.exportMovies(movies, [&]() { ui->exportProgress->setValue(++processedCount); });

        saveCsvToFile("movies", exportDir, csv);
    }
    // TV shows ----------------------------------------
    {
        const QVector<TvShow*>& tvShows = Manager::instance()->tvShowModel()->tvShows();

        // Export with a progress bar (even though it may be so fast that it's not noticable)
        CsvTvExport tvExporter(getFields<CsvTvExport::TvField>(ui->tvShowDetailsToExport));
        tvExporter.setSeparator(separator);
        tvExporter.setReplacement(replacement);

        int processedCount = 0;
        ui->exportProgress->setRange(0, tvShows.size());

        QString csv = tvExporter.exportTvShows(tvShows, [&]() { ui->exportProgress->setValue(++processedCount); });

        saveCsvToFile("tv_shows", exportDir, csv);
    }
    // ------------------------------------------

    ui->btnExport->setEnabled(true);
}

void CsvExportDialog::initializeItems()
{
    using namespace mediaelch;


    {
        using Field = CsvMovieExport::MovieField;
        ui->movieDetailsToExport->clear();

        const auto addField = [this](Field field, const QString& name) {
            auto* item = new QListWidgetItem(name, ui->movieDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
        };

        addField(Field::Imdbid, tr("IMDb ID"));
        addField(Field::Tmdbid, tr("TMDb ID"));
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
        addField(Field::MovieSet, tr("Movie set"));
    }
    {
        using Field = CsvTvExport::TvField;

        ui->tvShowDetailsToExport->clear();

        const auto addField = [this](Field field, const QString& name) {
            auto* item = new QListWidgetItem(name, ui->tvShowDetailsToExport);
            item->setData(Qt::UserRole, static_cast<int>(field));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
        };

        addField(Field::ShowImdbId, tr("TV show IMDb ID"));
        addField(Field::ShowTmdbId, tr("TV show TMDb ID"));
        addField(Field::ShowTvDbId, tr("TV show TheTvDb ID"));
        addField(Field::ShowTvMazeId, tr("TV show TVmaze ID"));
        addField(Field::ShowTitle, tr("TV show title"));
        addField(Field::ShowFirstAired, tr("TV show first aired"));
        addField(Field::ShowNetwork, tr("TV show network"));
        addField(Field::ShowGenres, tr("TV show genres"));
        addField(Field::ShowRuntime, tr("TV show runtime"));
        addField(Field::ShowRatings, tr("TV show ratings"));
        addField(Field::ShowUserRating, tr("TV show user rating"));
        addField(Field::EpisodeSeason, tr("Episode season"));
        addField(Field::EpisodeNumber, tr("Episode number"));
        addField(Field::EpisodeImdbId, tr("Episode IMDb ID"));
        addField(Field::EpisodeTmdbId, tr("Episode TMDb ID"));
        addField(Field::EpisodeTvDbId, tr("Episode TheTvDb ID"));
        addField(Field::EpisodeTvMazeId, tr("Episode TVmaze ID"));
        addField(Field::EpisodeFirstAired, tr("Episode first aired"));
        addField(Field::EpisodeTitle, tr("Episode title"));
        addField(Field::EpisodeOverview, tr("Episode overview"));
        addField(Field::EpisodeUserRating, tr("Episode user rating"));
        addField(Field::EpisodeDirectors, tr("Episode directors"));
        addField(Field::EpisodeWriters, tr("Episode writers"));
        addField(Field::EpisodeActors, tr("Episode actors"));
    }
}

bool CsvExportDialog::saveCsvToFile(const QString& type, QDir exportDir, const QString& csv)
{
    // Write the file. This may take a while (for large files with a few MB) so reset the
    // progress bar to the "marquee style".
    ui->exportProgress->setRange(0, 0);

    QFile file(exportDir.path() + "/" + defaultCsvFileName(type));
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        // UTF-8 BOM required for e.g. Excel
        out.setCodec("UTF-8");
        out.setGenerateByteOrderMark(true);
        out << csv;
        // flush before closing the file or the data won't be written
        out.flush();
        file.close();

        if (out.status() == QTextStream::Ok) {
            ui->lblMessage->setSuccessMessage(tr("Export completed."));
            qInfo() << "[CsvExport] Finished successfully";

        } else {
            ui->lblMessage->setErrorMessage(tr("Export failed. Could not write CSV."));
            qInfo() << "[CsvExport] Failed";
        }

    } else {
        ui->lblMessage->setErrorMessage(tr("Export failed. File could not be opened for writing."));
        qInfo() << "[CsvExport] Failed: Could not open file";
    }

    ui->exportProgress->setRange(0, 1);
    ui->exportProgress->setValue(1);
}

QString CsvExportDialog::defaultCsvFileName(const QString& type) const
{
    return QStringLiteral("MediaElch_%1_%2.csv") //
        .arg(type, QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
}
