#pragma once

#include "globals/Globals.h"

#include <QSettings>
#include <QVector>

class DirectorySettings
{
public:
    void loadSettings();
    void saveSettings();
    void setQSettings(QSettings* settings) { m_settings = settings; }

    const QVector<SettingsDir>& movieDirectories() const;
    const QVector<SettingsDir>& tvShowDirectories() const;
    const QVector<SettingsDir>& concertDirectories() const;
    const QVector<SettingsDir>& downloadDirectories() const;
    const QVector<SettingsDir>& musicDirectories() const;

    void setMovieDirectories(QVector<SettingsDir> dirs);
    void setTvShowDirectories(QVector<SettingsDir> dirs);
    void setConcertDirectories(QVector<SettingsDir> dirs);
    void setDownloadDirectories(QVector<SettingsDir> dirs);
    void setMusicDirectories(QVector<SettingsDir> dirs);

private:
    QSettings* m_settings = nullptr;

    QVector<SettingsDir> m_movieDirectories;
    QVector<SettingsDir> m_tvShowDirectories;
    QVector<SettingsDir> m_concertDirectories;
    QVector<SettingsDir> m_downloadDirectories;
    QVector<SettingsDir> m_musicDirectories;
};
