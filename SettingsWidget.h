#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QSettings>
#include <QTableWidgetItem>
#include <QWidget>
#include "Globals.h"

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = 0);
    ~SettingsWidget();
    static SettingsWidget *instance();
    void loadSettings();
    QSize mainWindowSize();
    QPoint mainWindowPosition();
    QList<SettingsDir> movieDirectories();
    QList<SettingsDir> tvShowDirectories();
    bool firstTime();
    int mediaCenterInterface();
    QString xbmcMysqlHost();
    QString xbmcMysqlDatabase();
    QString xbmcMysqlUser();
    QString xbmcMysqlPassword();
    QString xbmcSqliteDatabase();
    QString xbmcThumbnailPath();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);

public slots:
    void saveSettings();

private slots:
    void addMovieDir();
    void removeMovieDir();
    void movieListRowChanged(int currentRow);
    void addTvShowDir();
    void removeTvShowDir();
    void tvShowListRowChanged(int currentRow);
    void movieMediaCenterPathChanged(QTableWidgetItem *item);
    void tvShowMediaCenterPathChanged(QTableWidgetItem *item);
    void onMediaCenterXbmcXmlSelected();
    void onMediaCenterXbmcMysqlSelected();
    void onMediaCenterXbmcSqliteSelected();
    void onChooseMediaCenterXbmcSqliteDatabase();
    void onChooseXbmcThumbnailPath();

private:
    Ui::SettingsWidget *ui;
    QSettings m_settings;
    QList<SettingsDir> m_movieDirectories;
    QList<SettingsDir> m_tvShowDirectories;
    QSize m_mainWindowSize;
    QPoint m_mainWindowPosition;
    bool m_firstTime;
    int m_mediaCenterInterface;
    QString m_xbmcMysqlHost;
    QString m_xbmcMysqlDatabase;
    QString m_xbmcMysqlUser;
    QString m_xbmcMysqlPassword;
    QString m_xbmcSqliteDatabase;
    QString m_xbmcThumbnailPath;
    static SettingsWidget *m_instance;

    void setXbmcThumbnailPathEnabled(bool enabled);
};

#endif // SETTINGSWIDGET_H
