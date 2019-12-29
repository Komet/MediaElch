#include "settings/DirectorySettings.h"

#include <QDir>

void DirectorySettings::loadSettings()
{
    const auto loadDirectory = [&](const char* settingsKey, QVector<SettingsDir>& directories) {
        directories.clear();
        const int size = m_settings->beginReadArray(settingsKey);
        for (int i = 0; i < size; ++i) {
            m_settings->setArrayIndex(i);
            SettingsDir dir;
            dir.path.setPath(QDir::toNativeSeparators(m_settings->value("path").toString()));
            dir.separateFolders = m_settings->value("sepFolders", false).toBool();
            dir.autoReload = m_settings->value("autoReload", false).toBool();
            directories.append(dir);
        }
        m_settings->endArray();
    };

    loadDirectory("Directories/Movies", m_movieDirectories);
    loadDirectory("Directories/TvShows", m_tvShowDirectories);
    loadDirectory("Directories/Concerts", m_concertDirectories);
    loadDirectory("Directories/Downloads", m_downloadDirectories);
    loadDirectory("Directories/Music", m_musicDirectories);
}

void DirectorySettings::saveSettings()
{
    const auto saveDirectory = [&](const char* settingsKey, QVector<SettingsDir>& directories) {
        m_settings->beginWriteArray(settingsKey);
        const int size = directories.count();
        for (int i = 0; i < size; ++i) {
            m_settings->setArrayIndex(i);
            m_settings->setValue("path", directories.at(i).path.path());
            m_settings->setValue("sepFolders", directories.at(i).separateFolders);
            m_settings->setValue("autoReload", directories.at(i).autoReload);
        }
        m_settings->endArray();
    };

    saveDirectory("Directories/Movies", m_movieDirectories);
    saveDirectory("Directories/Concerts", m_concertDirectories);
    saveDirectory("Directories/Downloads", m_downloadDirectories);
    saveDirectory("Directories/Music", m_musicDirectories);

    m_settings->beginWriteArray("Directories/TvShows");
    for (int i = 0, n = m_tvShowDirectories.count(); i < n; ++i) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("path", m_tvShowDirectories.at(i).path.path());
        m_settings->setValue("autoReload", m_tvShowDirectories.at(i).autoReload);
    }
    m_settings->endArray();
}

const QVector<SettingsDir>& DirectorySettings::movieDirectories() const
{
    return m_movieDirectories;
}

const QVector<SettingsDir>& DirectorySettings::tvShowDirectories() const
{
    return m_tvShowDirectories;
}

const QVector<SettingsDir>& DirectorySettings::concertDirectories() const
{
    return m_concertDirectories;
}

const QVector<SettingsDir>& DirectorySettings::musicDirectories() const
{
    return m_musicDirectories;
}

const QVector<SettingsDir>& DirectorySettings::downloadDirectories() const
{
    return m_downloadDirectories;
}

void DirectorySettings::setMovieDirectories(QVector<SettingsDir> dirs)
{
    m_movieDirectories = dirs;
}

void DirectorySettings::setTvShowDirectories(QVector<SettingsDir> dirs)
{
    m_tvShowDirectories = dirs;
}

void DirectorySettings::setConcertDirectories(QVector<SettingsDir> dirs)
{
    m_concertDirectories = dirs;
}

void DirectorySettings::setMusicDirectories(QVector<SettingsDir> dirs)
{
    m_musicDirectories = dirs;
}

void DirectorySettings::setDownloadDirectories(QVector<SettingsDir> dirs)
{
    m_downloadDirectories = dirs;
}
