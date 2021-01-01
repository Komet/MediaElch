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
    QString saveAs = QFileDialog::getSaveFileName(
        this, tr("Save File"), QDir::homePath() + defaultCsvFileName(), tr("CSV (*.csv *.txt)"));

    // ------------------------------------------
    const QVector<Movie*>& movies = Manager::instance()->movieModel()->movies();

    // Export with a progress bar (even though it may be so fast that it's not noticable)
    CsvMovieExport csvExporter(getMovieFields());
    csvExporter.setSeparator(separator);
    csvExporter.setReplacement(replacement);

    int processedCount = 0;
    ui->exportProgress->setRange(0, movies.size());

    QString csv = csvExporter.exportMovies(movies, [&]() { ui->exportProgress->setValue(++processedCount); });

    // ------------------------------------------

    // Write the file. This may take a while (for large files with a few MB) so reset the
    // progress bar to the "marquee style".
    ui->exportProgress->setRange(0, 0);

    QFile file(saveAs);
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

    ui->btnExport->setEnabled(true);
}

void CsvExportDialog::initializeItems()
{
    using namespace mediaelch;

    ui->detailsToExport->clear();

    const auto addField = [this](CsvMovieExport::MovieField field, const QString& name) {
        auto* item = new QListWidgetItem(name, ui->detailsToExport);
        item->setData(Qt::UserRole, static_cast<int>(field));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    };

    using Field = CsvMovieExport::MovieField;
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

QVector<mediaelch::CsvMovieExport::MovieField> CsvExportDialog::getMovieFields() const
{
    using namespace mediaelch;
    QVector<CsvMovieExport::MovieField> fields;

    for (int i = 0; i < ui->detailsToExport->count(); ++i) {
        const QListWidgetItem* item = ui->detailsToExport->item(i);
        if (item->checkState() == Qt::Unchecked) {
            continue;
        }

        bool ok = false;
        const int value = item->data(Qt::UserRole).toInt(&ok);
        if (ok) {
            fields.push_back(static_cast<CsvMovieExport::MovieField>(value));
        }
    }
    return fields;
}

QString CsvExportDialog::defaultCsvFileName() const
{
    return QStringLiteral("/MediaElch_Export_%1.csv") //
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
}
