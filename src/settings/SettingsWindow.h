#pragma once

#include "globals/Globals.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QListWidgetItem>
#include <QMainWindow>

namespace Ui {
class SettingsWindow;
}

class ExportTemplate;
class MovieScraperInterface;
class Settings;

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow() override;

public slots:
    void show();

signals:
    void sigSaved();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAction();
    void chooseDirToAdd();
    void addDir(QString dir,
        bool separateFolders = false,
        bool autoReload = false,
        SettingsDirType dirType = SettingsDirType::Movies);
    void removeDir();
    void organize();
    void dirListRowChanged(int currentRow);
    void onComboMovieSetArtworkChanged();
    void onChooseMovieSetArtworkDir();
    void onUseProxy();
    void onSave();
    void onCancel();
    void onTemplatesLoaded(QVector<ExportTemplate *> templates);
    void onTemplateInstalled(ExportTemplate *exportTemplate, bool success);
    void onTemplateUninstalled(ExportTemplate *exportTemplate, bool success);
    void onChooseUnrar();
    void onChooseMakeMkvCon();
    void onDirTypeChanged(QComboBox *comboBox = nullptr);
    void onShowAdultScrapers();

private:
    Ui::SettingsWindow *ui;
    Settings *m_settings;
    QMap<const MovieScraperInterface *, int> m_scraperRows;
    QColor m_buttonColor;
    QColor m_buttonActiveColor;

    void loadSettings();
    void saveSettings();
    void loadRemoteTemplates();
    QComboBox *comboForMovieScraperInfo(MovieScraperInfos info);
    QString titleForMovieScraperInfo(MovieScraperInfos info);
    QComboBox *comboForTvScraperInfo(TvShowScraperInfos info);
    QString titleForTvScraperInfo(TvShowScraperInfos info);
};
