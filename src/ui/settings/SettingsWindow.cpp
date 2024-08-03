#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "settings/DataFile.h"
#include "settings/Settings.h"
#include "ui/UiUtils.h"
#include "ui/notifications/NotificationBox.h"
#include "ui/small_widgets/PlaceholderLineEdit.h"

#include <QAction>
#include <QLineEdit>

namespace {

static constexpr char moduleName[] = "ui";
static const Settings::Key KEY_SETTINGS_WINDOW_SIZE(moduleName, "SettingsWindowSize");
static const Settings::Key KEY_SETTINGS_WINDOW_POSITION(moduleName, "SettingsWindowPosition");

QPoint fixWindowPosition(QPoint p)
{
    p.setX(qMax(0, p.x()));
    p.setY(qMax(0, p.y()));
    return p;
}

} // namespace

SettingsWindowConfiguration::SettingsWindowConfiguration(Settings& settings) : m_settings{settings}
{
}

void SettingsWindowConfiguration::init()
{
    m_settings.setDefaultValue(KEY_SETTINGS_WINDOW_SIZE, QVariant{});
    m_settings.setDefaultValue(KEY_SETTINGS_WINDOW_POSITION, QVariant{});
}

QSize SettingsWindowConfiguration::settingsWindowSize()
{
    return m_settings.value(KEY_SETTINGS_WINDOW_SIZE).toSize();
}

void SettingsWindowConfiguration::setSettingsWindowSize(QSize settingsWindowSize)
{
    m_settings.setValue(KEY_SETTINGS_WINDOW_SIZE, settingsWindowSize);
}

QPoint SettingsWindowConfiguration::settingsWindowPosition()
{
    return fixWindowPosition(m_settings.value(KEY_SETTINGS_WINDOW_POSITION).toPoint());
}

void SettingsWindowConfiguration::setSettingsWindowPosition(QPoint settingsWindowPosition)
{
    m_settings.setValue(KEY_SETTINGS_WINDOW_POSITION, settingsWindowPosition);
}

SettingsWindow::SettingsWindow(Settings& settings, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow),
    m_settings{settings},
    m_config{SettingsWindowConfiguration(settings)},
    m_buttonColor{QColor(128, 129, 132)},
    m_buttonActiveColor{QColor(70, 155, 198)}
{
    ui->setupUi(this);
    m_config.init();

    ui->settingsTabs->setCurrentIndex(0);

    ui->globalSettings->setSettings(m_settings);
    ui->exportSettings->setSettings(m_settings);
    ui->importSettings->setSettings(m_settings);
    ui->scraperSettings->setSettings(m_settings);
    ui->tvShowSettings->setSettings(m_settings);
    ui->movieSettings->setSettings(m_settings);
    ui->musicSettings->setSettings(m_settings);
    ui->concertSettings->setSettings(m_settings);
    ui->networkSettings->setSettings(m_settings);

    ui->kodiSettings->init(Manager::instance()->kodiSettings());

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
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::show()
{
    if (!isVisible()) {
        loadSettings();
        m_settings.beginTransaction();

        ui->exportSettings->show();

        if (m_config.settingsWindowSize().isValid() && !m_config.settingsWindowPosition().isNull()) {
            move(m_config.settingsWindowPosition());
            resize(m_config.settingsWindowSize());
        }
    }
    QMainWindow::show();
    activateWindow();
    raise();
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    m_config.setSettingsWindowSize(size());
    m_config.setSettingsWindowPosition(pos());

#ifdef Q_OS_MAC
    saveSettings();
    emit sigSaved();
#else
    if (!m_saveCloseHandled) {
        m_settings.abortTransaction();
        m_settings.loadSettings();
    }
#endif

    QMainWindow::closeEvent(event);
}

void SettingsWindow::onSave()
{
    saveSettings();
    m_saveCloseHandled = true;
    close();
    emit sigSaved();
}

void SettingsWindow::onCancel()
{
    m_settings.abortTransaction();
    m_settings.loadSettings();
    m_saveCloseHandled = true;
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
    ui->globalSettings->loadSettings();
    ui->exportSettings->loadSettings();
    ui->importSettings->loadSettings();
    ui->scraperSettings->loadSettings();
    ui->tvShowSettings->loadSettings();
    ui->movieSettings->loadSettings();
    ui->musicSettings->loadSettings();
    ui->concertSettings->loadSettings();
    ui->networkSettings->loadSettings();

    const auto work = [this](auto* lineEdit) {
        if (lineEdit->property("dataFileType").isNull()) {
            return;
        }
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QVector<DataFile> dataFiles = m_settings.dataFiles(dataFileType);
        QStringList filenames;
        for (const DataFile& dataFile : dataFiles) {
            filenames << dataFile.fileName();
        }
        lineEdit->setText(filenames.join(","));
    };
    for (auto* lineEdit : findChildren<QLineEdit*>()) {
        work(lineEdit);
    }
    for (auto* lineEdit : findChildren<PlaceholderLineEdit*>()) {
        work(lineEdit);
    }
}

void SettingsWindow::saveSettings()
{
    QVector<DataFile> dataFiles;
    const auto work = [&dataFiles](auto* lineEdit) {
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
        work(lineEdit);
    }
    for (auto* lineEdit : findChildren<PlaceholderLineEdit*>()) {
        work(lineEdit);
    }
    m_settings.setDataFiles(dataFiles);

    ui->globalSettings->saveSettings();
    ui->exportSettings->saveSettings();
    ui->importSettings->saveSettings();
    ui->scraperSettings->saveSettings();
    ui->tvShowSettings->saveSettings();
    ui->movieSettings->saveSettings();
    ui->musicSettings->saveSettings();
    ui->concertSettings->saveSettings();
    ui->networkSettings->saveSettings();

    m_settings.saveSettings();
    m_settings.commitTransaction();

    auto* manager = Manager::instance();
    auto& dirs = m_settings.directorySettings();
    manager->movieFileSearcher()->setMovieDirectories(dirs.movieDirectories());
    manager->tvShowFileSearcher()->setTvShowDirectories(dirs.tvShowDirectories());
    manager->concertFileSearcher()->setConcertDirectories(dirs.concertDirectories());
    manager->musicFileSearcher()->setMusicDirectories(dirs.musicDirectories());
    NotificationBox::instance()->showSuccess(tr("Settings saved"));
}
