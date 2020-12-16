#include "ui/settings/MusicSettingsWidget.h"
#include "ui_MusicSettingsWidget.h"

#include "settings/DataFile.h"
#include "settings/Settings.h"

MusicSettingsWidget::MusicSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MusicSettingsWidget)
{
    ui->setupUi(this);

    ui->artistFanart->setProperty("dataFileType", static_cast<int>(DataFileType::ArtistFanart));
    ui->artistLogo->setProperty("dataFileType", static_cast<int>(DataFileType::ArtistLogo));
    ui->artistThumb->setProperty("dataFileType", static_cast<int>(DataFileType::ArtistThumb));
    ui->albumThumb->setProperty("dataFileType", static_cast<int>(DataFileType::AlbumThumb));
    ui->albumDiscArt->setProperty("dataFileType", static_cast<int>(DataFileType::AlbumCdArt));
}

MusicSettingsWidget::~MusicSettingsWidget()
{
    delete ui;
}

void MusicSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void MusicSettingsWidget::loadSettings()
{
    for (auto* lineEdit : findChildren<QLineEdit*>()) {
        if (lineEdit->property("dataFileType").isNull()) {
            continue;
        }
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QVector<DataFile> dataFiles = m_settings->dataFiles(dataFileType);
        QStringList filenames;
        for (const DataFile& dataFile : dataFiles) {
            filenames << dataFile.fileName();
        }
        lineEdit->setText(filenames.join(","));
    }

    ui->artistExtraFanarts->setValue(m_settings->extraFanartsMusicArtists());
}

void MusicSettingsWidget::saveSettings()
{
    QVector<DataFile> dataFiles;
    for (QLineEdit* lineEdit : findChildren<QLineEdit*>()) {
        if (lineEdit->property("dataFileType").isNull()) {
            continue;
        }
        int pos = 0;
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QStringList filenames = lineEdit->text().split(",", ElchSplitBehavior::SkipEmptyParts);
        for (const QString& filename : filenames) {
            DataFile df(dataFileType, filename.trimmed(), pos++);
            dataFiles << df;
        }
    }

    m_settings->setExtraFanartsMusicArtists(ui->artistExtraFanarts->value());
}
