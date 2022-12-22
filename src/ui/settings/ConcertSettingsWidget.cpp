#include "ui/settings/ConcertSettingsWidget.h"
#include "ui_ConcertSettingsWidget.h"

#include "settings/Settings.h"

#include <QLineEdit>

ConcertSettingsWidget::ConcertSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ConcertSettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->lblConcertIntro->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->lblConcertIntro->setFont(smallFont);
    ui->lblConcertPlaceholderIntro->setFont(smallFont);
#endif

    ui->concertNfo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertNfo));
    ui->concertPoster->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertPoster));
    ui->concertBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertBackdrop));
    ui->concertLogo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertLogo));
    ui->concertClearArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertClearArt));
    ui->concertDiscArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertCdArt));

    for (auto* lineEdit : findChildren<PlaceholderLineEdit*>()) {
        lineEdit->setPlaceholders({"baseFileName"});
    }
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
    const auto loadLineEdit = [this](auto* lineEdit) {
        if (lineEdit->property("dataFileType").isNull()) {
            return;
        }
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QVector<DataFile> dataFiles = m_settings->dataFiles(dataFileType);
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

void ConcertSettingsWidget::saveSettings()
{
    QVector<DataFile> dataFiles;
    const auto storeLineEdit = [&dataFiles](auto* lineEdit) {
        if (lineEdit->property("dataFileType").isNull()) {
            return;
        }
        int pos = 0;
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QStringList filenames = lineEdit->text().split(",", ElchSplitBehavior::SkipEmptyParts);
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
}
