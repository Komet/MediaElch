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

#ifdef Q_OS_MAC
    QFont smallFont = ui->label_48->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->label_48->setFont(smallFont);
    ui->label_49->setFont(smallFont);
#endif

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

    Helper::removeFocusRect(ui->settingsTabs->widget(9));

    // clang-format off
    connect(ui->chkUseProxy,            &QAbstractButton::clicked, this, &SettingsWindow::onUseProxy);
    connect(ui->btnCancel,              &QAbstractButton::clicked, this, &SettingsWindow::onCancel);
    connect(ui->btnSave,                &QAbstractButton::clicked, this, &SettingsWindow::onSave);
    // clang-format on

    ui->concertNfo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertNfo));
    ui->concertPoster->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertPoster));
    ui->concertBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertBackdrop));
    ui->concertLogo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertLogo));
    ui->concertClearArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertClearArt));
    ui->concertDiscArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertCdArt));

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
    auto triggeredAction = static_cast<QAction*>(sender());
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

    // Proxy
    const auto& netSettings = m_settings->networkSettings();
    ui->chkUseProxy->setChecked(netSettings.useProxy());
    ui->proxyType->setCurrentIndex(netSettings.proxyType());
    ui->proxyHost->setText(netSettings.proxyHost());
    ui->proxyPort->setValue(netSettings.proxyPort());
    ui->proxyUsername->setText(netSettings.proxyUsername());
    ui->proxyPassword->setText(netSettings.proxyPassword());
    onUseProxy();

    for (auto lineEdit : findChildren<QLineEdit*>()) {
        if (lineEdit->property("dataFileType").isNull()) {
            continue;
        }
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QVector<DataFile> dataFiles = m_settings->dataFiles(dataFileType);
        QStringList filenames;
        for (DataFile dataFile : dataFiles) {
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
        QStringList filenames = lineEdit->text().split(",", QString::SkipEmptyParts);
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

    // Proxy
    m_settings->networkSettings().setUseProxy(ui->chkUseProxy->isChecked());
    m_settings->networkSettings().setProxyType(ui->proxyType->currentIndex());
    m_settings->networkSettings().setProxyHost(ui->proxyHost->text());
    m_settings->networkSettings().setProxyPort(ui->proxyPort->value());
    m_settings->networkSettings().setProxyUsername(ui->proxyUsername->text());
    m_settings->networkSettings().setProxyPassword(ui->proxyPassword->text());

    m_settings->saveSettings();

    auto* manager = Manager::instance();
    auto& dirs = m_settings->directorySettings();
    manager->movieFileSearcher()->setMovieDirectories(dirs.movieDirectories());
    manager->tvShowFileSearcher()->setTvShowDirectories(dirs.tvShowDirectories());
    manager->concertFileSearcher()->setConcertDirectories(dirs.concertDirectories());
    manager->musicFileSearcher()->setMusicDirectories(dirs.musicDirectories());
    NotificationBox::instance()->showMessage(tr("Settings saved"));
}

void SettingsWindow::onUseProxy()
{
    bool enabled = ui->chkUseProxy->isChecked();
    ui->proxyType->setEnabled(enabled);
    ui->proxyHost->setEnabled(enabled);
    ui->proxyPort->setEnabled(enabled);
    ui->proxyUsername->setEnabled(enabled);
    ui->proxyPassword->setEnabled(enabled);
}
