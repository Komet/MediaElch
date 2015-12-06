#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QCloseEvent>
#include <QComboBox>
#include <QListWidgetItem>
#include <QMainWindow>

#include "data/ConcertScraperInterface.h"
#include "data/ScraperInterface.h"
#include "data/TvScraperInterface.h"
#include "export/ExportTemplate.h"
#include "globals/Globals.h"
#include "plugins/PluginInterface.h"
#include "plugins/PluginManager.h"
#include "plugins/PluginManagerDialog.h"
#include "settings/Settings.h"

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

public slots:
    void show();

signals:
    void sigSaved();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void onAction();
    void chooseDirToAdd();
    void addDir(QString dir, bool separateFolders = false, bool autoReload = false, SettingsDirType dirType = DirTypeMovies);
    void removeDir();
    void organize();
    void dirListRowChanged(int currentRow);
    void onComboMovieSetArtworkChanged();
    void onChooseMovieSetArtworkDir();
    void onUseProxy();
    void onSave();
    void onCancel();
    void onTemplatesLoaded(QList<ExportTemplate*> templates);
    void onTemplateInstalled(ExportTemplate *exportTemplate, bool success);
    void onTemplateUninstalled(ExportTemplate *exportTemplate, bool success);
    void onChooseUnrar();
    void onChooseMakeMkvCon();
    void onDirTypeChanged(QComboBox *comboBox = 0);
    void onShowAdultScrapers();
    void onPluginListUpdated(QList<PluginManager::Plugin> plugins);
    void onPluginActivated(QListWidgetItem *item);
    void onInstallPlugin();
    void onUninstallPlugin();
    void onUpdatePlugin();

private:
    Ui::SettingsWindow *ui;
    Settings *m_settings;
    QMap<ScraperInterface*, int> m_scraperRows;
    QMap<int, PluginInterface*> m_plugins;
    PluginManagerDialog *m_pluginDialog;
    bool m_pluginsInstallable;
    QColor m_buttonColor;
    QColor m_buttonActiveColor;

    void loadSettings();
    void saveSettings();
    void loadRemoteTemplates();
    QComboBox* comboForMovieScraperInfo(const int &info);
    QString titleForMovieScraperInfo(const int &info);
    QComboBox* comboForTvScraperInfo(const int &info);
    QString titleForTvScraperInfo(const int &info);
    void setPluginActionsEnabled(const bool &enabled);
};

#endif // SETTINGSWINDOW_H
