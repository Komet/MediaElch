#include "ui/settings/ConcertSettingsWidget.h"
#include "ui_ConcertSettingsWidget.h"

#include "settings/Settings.h"

ConcertSettingsWidget::ConcertSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ConcertSettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->label_48->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->label_48->setFont(smallFont);
    ui->label_49->setFont(smallFont);
#endif

    ui->concertNfo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertNfo));
    ui->concertPoster->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertPoster));
    ui->concertBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertBackdrop));
    ui->concertLogo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertLogo));
    ui->concertClearArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertClearArt));
    ui->concertDiscArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertCdArt));
}

ConcertSettingsWidget::~ConcertSettingsWidget()
{
    delete ui;
}

void ConcertSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void ConcertSettingsWidget::loadSettings()
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
}

void ConcertSettingsWidget::saveSettings()
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
}
