#include "ui/settings/MovieSettingsWidget.h"
#include "ui_MovieSettingsWidget.h"

#include "settings/DataFile.h"
#include "settings/Settings.h"

#include <QFileDialog>

MovieSettingsWidget::MovieSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MovieSettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->lblFilenameDescription->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->lblFilenameDescription->setFont(smallFont);
    ui->lblPlaceholders->setFont(smallFont);
#endif

    ui->comboMovieSetArtwork->addItem(
        tr("Artwork next to movies"), static_cast<int>(MovieSetArtworkType::ArtworkNextToMovies));
    ui->comboMovieSetArtwork->addItem(
        tr("Separate artwork directory"), static_cast<int>(MovieSetArtworkType::SeparateArtworkFolder));

    connect(ui->comboMovieSetArtwork,
        elchOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &MovieSettingsWidget::onComboMovieSetArtworkChanged);

    connect(ui->btnMovieSetArtworkDir,
        &QAbstractButton::clicked, //
        this,
        &MovieSettingsWidget::onChooseMovieSetArtworkDir);

    ui->movieNfo->setProperty("dataFileType", static_cast<int>(DataFileType::MovieNfo));
    ui->moviePoster->setProperty("dataFileType", static_cast<int>(DataFileType::MoviePoster));
    ui->movieBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::MovieBackdrop));
    ui->movieCdArt->setProperty("dataFileType", static_cast<int>(DataFileType::MovieCdArt));
    ui->movieClearArt->setProperty("dataFileType", static_cast<int>(DataFileType::MovieClearArt));
    ui->movieLogo->setProperty("dataFileType", static_cast<int>(DataFileType::MovieLogo));
    ui->movieBanner->setProperty("dataFileType", static_cast<int>(DataFileType::MovieBanner));
    ui->movieThumb->setProperty("dataFileType", static_cast<int>(DataFileType::MovieThumb));
    ui->movieSetPosterFileName->setProperty("dataFileType", static_cast<int>(DataFileType::MovieSetPoster));
    ui->movieSetFanartFileName->setProperty("dataFileType", static_cast<int>(DataFileType::MovieSetBackdrop));

    const QStringList placeholders({"baseFileName"});
    ui->movieNfo->setPlaceholders(placeholders);
    ui->moviePoster->setPlaceholders(placeholders);
    ui->movieBackdrop->setPlaceholders(placeholders);
    ui->movieCdArt->setPlaceholders(placeholders);
    ui->movieClearArt->setPlaceholders(placeholders);
    ui->movieLogo->setPlaceholders(placeholders);
    ui->movieBanner->setPlaceholders(placeholders);
    ui->movieThumb->setPlaceholders(placeholders);
    ui->movieSetPosterFileName->setPlaceholders({"setName"});
    ui->movieSetFanartFileName->setPlaceholders({"setName"});
}

MovieSettingsWidget::~MovieSettingsWidget()
{
    delete ui;
}

void MovieSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void MovieSettingsWidget::loadSettings()
{
    ui->usePlotForOutline->setChecked(m_settings->usePlotForOutline());
    ui->ignoreDuplicateOriginalTitle->setChecked(m_settings->ignoreDuplicateOriginalTitle());

    // Movie set artwork
    for (int i = 0, n = ui->comboMovieSetArtwork->count(); i < n; ++i) {
        if (MovieSetArtworkType(ui->comboMovieSetArtwork->itemData(i).toInt()) == m_settings->movieSetArtworkType()) {
            ui->comboMovieSetArtwork->setCurrentIndex(i);
            break;
        }
    }
    ui->movieSetArtworkDir->setText(m_settings->movieSetArtworkDirectory().toNativePathString());
    onComboMovieSetArtworkChanged(ui->comboMovieSetArtwork->currentIndex());

    const auto loadLineEdit = [this](auto* lineEdit) {
        if (lineEdit->property("dataFileType").isNull()) {
            return;
        }
        bool ok = false;
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt(&ok));
        if (!ok) {
            return;
        }
        const QVector<DataFile> dataFiles = m_settings->dataFiles(dataFileType);
        QStringList filenames;
        for (const DataFile& dataFile : dataFiles) {
            filenames << dataFile.fileName();
        }
        lineEdit->setText(filenames.join(","));
    };
    for (auto* lineEdit : findChildren<QLineEdit*>()) {
        loadLineEdit(lineEdit);
    }
    for (auto* lineEdit : findChildren<PlaceholderLineEdit*>()) {
        loadLineEdit(lineEdit);
    }
}

void MovieSettingsWidget::saveSettings()
{
    QVector<DataFile> dataFiles;
    const auto storeLineEdit = [&dataFiles](auto* lineEdit) {
        if (lineEdit->property("dataFileType").isNull()) {
            return;
        }
        int pos = 0;
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        const QStringList filenames = lineEdit->text().split(",", ElchSplitBehavior::SkipEmptyParts);
        for (const QString& filename : filenames) {
            DataFile df(dataFileType, filename.trimmed(), pos++);
            dataFiles << df;
        }
    };

    for (auto* lineEdit : findChildren<QLineEdit*>()) {
        storeLineEdit(lineEdit);
    }
    for (auto* lineEdit : findChildren<PlaceholderLineEdit*>()) {
        storeLineEdit(lineEdit);
    }

    m_settings->setUsePlotForOutline(ui->usePlotForOutline->isChecked());
    m_settings->setIgnoreDuplicateOriginalTitle(ui->ignoreDuplicateOriginalTitle->isChecked());

    // Movie set artwork
    m_settings->setMovieSetArtworkType(static_cast<MovieSetArtworkType>(
        ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt()));
    m_settings->setMovieSetArtworkDirectory(mediaelch::DirectoryPath(ui->movieSetArtworkDir->text()));
}

void MovieSettingsWidget::onComboMovieSetArtworkChanged(int comboIndex)
{
    MovieSetArtworkType value = MovieSetArtworkType(ui->comboMovieSetArtwork->itemData(comboIndex).toInt());
    ui->btnMovieSetArtworkDir->setEnabled(value == MovieSetArtworkType::SeparateArtworkFolder);
    ui->movieSetArtworkDir->setEnabled(value == MovieSetArtworkType::SeparateArtworkFolder);

    switch (value) {
    case MovieSetArtworkType::ArtworkNextToMovies: {
        ui->movieSetPosterFileName->setText("<setName>-folder.jpg");
        ui->movieSetFanartFileName->setText("<setName>-fanart.jpg");
        break;
    }
    case MovieSetArtworkType::SeparateArtworkFolder: {
        ui->movieSetPosterFileName->setText("folder.jpg");
        ui->movieSetFanartFileName->setText("fanart.jpg");
        break;
    }
    }
}

void MovieSettingsWidget::onChooseMovieSetArtworkDir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Choose a directory where your movie set artwork is stored"), QDir::homePath());
    if (!dir.isEmpty()) {
        ui->movieSetArtworkDir->setText(dir);
    }
}
