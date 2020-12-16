#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QAction>
#include <QDebug>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "settings/DataFile.h"
#include "settings/Settings.h"
#include "ui/notifications/NotificationBox.h"

SettingsWindow::SettingsWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow),
    m_buttonColor{QColor(128, 129, 132)},
    m_buttonActiveColor{QColor(70, 155, 198)}
{
    ui->setupUi(this);

    ui->settingsTabs->setCurrentIndex(0);

    m_settings = Settings::instance(this);
    ui->globalSettings->setSettings(*m_settings);
    ui->exportSettings->setSettings(*m_settings);
    ui->importSettings->setSettings(*m_settings);
    ui->scraperSettings->setSettings(*m_settings);
    ui->tvShowSettings->setSettings(*m_settings);
    ui->movieSettings->setSettings(*m_settings);
    ui->musicSettings->setSettings(*m_settings);
    ui->kodiSettings->setSettings(*m_settings);
    ui->concertSettings->setSettings(*m_settings);
    ui->networkSettings->setSettings(*m_settings);

    helper::removeFocusRect(ui->settingsTabs->widget(9));

    // clang-format off
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &SettingsWindow::onCancel);
    connect(ui->btnSave,   &QAbstractButton::clicked, this, &SettingsWindow::onSave);
    // clang-format on

#ifdef Q_OS_MAC
    ui->btnCancel->setVisible(false);
    ui->btnSave->setVisible(false);
    ui->horizontalSpacerButtons->setGeometry(QRect(0, 0, 1, 1));
#endif

    QPainter p;
    for (QAction* action : findChildren<QAction*>()) {
        if (!action->property("page").isValid()) {
            continue;
        }
        action->setIcon(Manager::instance()->iconFont()->icon(action->property("iconName").toString(), m_buttonColor));
    }
    ui->actionGlobal->setIcon(
        Manager::instance()->iconFont()->icon(ui->actionGlobal->property("iconName").toString(), m_buttonActiveColor));

    loadSettings();
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::show()
{
    ui->exportSettings->show();
    loadSettings();
    if (Settings::instance()->settingsWindowSize().isValid()
        && !Settings::instance()->settingsWindowPosition().isNull()) {
        move(Settings::instance()->settingsWindowPosition());
        resize(Settings::instance()->settingsWindowSize());
    }
    QMainWindow::show();
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);

#ifdef Q_OS_MAC
    saveSettings();
    emit sigSaved();
#endif

    Settings::instance()->setSettingsWindowSize(size());
    Settings::instance()->setSettingsWindowPosition(pos());
}

void SettingsWindow::onSave()
{
    saveSettings();
    close();
    emit sigSaved();
}

void SettingsWindow::onCancel()
{
    m_settings->loadSettings();
    close();
}

void SettingsWindow::onAction()
{
    auto* triggeredAction = dynamic_cast<QAction*>(sender());
    for (QAction* action : ui->toolBar->actions()) {
        action->setIcon(Manager::instance()->iconFont()->icon(action->property("iconName").toString(), m_buttonColor));
    }
    triggeredAction->setIcon(
        Manager::instance()->iconFont()->icon(triggeredAction->property("iconName").toString(), m_buttonActiveColor));
    ui->settingsTabs->setCurrentIndex(triggeredAction->property("page").toInt());
}

void SettingsWindow::loadSettings()
{
    m_settings->loadSettings();
    ui->globalSettings->loadSettings();
    ui->exportSettings->loadSettings();
    ui->importSettings->loadSettings();
    ui->scraperSettings->loadSettings();
    ui->tvShowSettings->loadSettings();
    ui->movieSettings->loadSettings();
    ui->musicSettings->loadSettings();
    ui->kodiSettings->loadSettings();
    ui->concertSettings->loadSettings();
    ui->networkSettings->loadSettings();

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

void SettingsWindow::saveSettings()
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
    m_settings->setDataFiles(dataFiles);

    ui->globalSettings->saveSettings();
    ui->exportSettings->saveSettings();
    ui->importSettings->saveSettings();
    ui->scraperSettings->saveSettings();
    ui->tvShowSettings->saveSettings();
    ui->movieSettings->saveSettings();
    ui->musicSettings->saveSettings();
    ui->kodiSettings->saveSettings();
    ui->concertSettings->saveSettings();
    ui->networkSettings->saveSettings();

    m_settings->saveSettings();

    auto* manager = Manager::instance();
    auto& dirs = m_settings->directorySettings();
    manager->movieFileSearcher()->setMovieDirectories(dirs.movieDirectories());
    manager->tvShowFileSearcher()->setTvShowDirectories(dirs.tvShowDirectories());
    manager->concertFileSearcher()->setConcertDirectories(dirs.concertDirectories());
    manager->musicFileSearcher()->setMusicDirectories(dirs.musicDirectories());
    NotificationBox::instance()->showSuccess(tr("Settings saved"));
}
