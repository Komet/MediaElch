#include "settings/DirectorySettings.h"

#include <QDir>

void DirectorySettings::loadSettings()
{
    // Movie Directories
    m_movieDirectories.clear();
    int moviesSize = m_settings->beginReadArray("Directories/Movies");
    for (int i = 0; i < moviesSize; ++i) {
        m_settings->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings->value("path").toString());
        dir.separateFolders = m_settings->value("sepFolders", false).toBool();
        dir.autoReload = m_settings->value("autoReload", false).toBool();
        m_movieDirectories.append(dir);
    }
    m_settings->endArray();

    // TV Show Directories
    m_tvShowDirectories.clear();
    int tvShowSize = m_settings->beginReadArray("Directories/TvShows");
    for (int i = 0; i < tvShowSize; ++i) {
        m_settings->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings->value("path").toString());
        dir.separateFolders = m_settings->value("sepFolders", false).toBool();
        dir.autoReload = m_settings->value("autoReload", false).toBool();
        m_tvShowDirectories.append(dir);
    }
    m_settings->endArray();

    // Concert Directories
    m_concertDirectories.clear();
    int concertsSize = m_settings->beginReadArray("Directories/Concerts");
    for (int i = 0; i < concertsSize; ++i) {
        m_settings->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings->value("path").toString());
        dir.separateFolders = m_settings->value("sepFolders", false).toBool();
        dir.autoReload = m_settings->value("autoReload", false).toBool();
        m_concertDirectories.append(dir);
    }
    m_settings->endArray();

    m_downloadDirectories.clear();
    int downloadsSize = m_settings->beginReadArray("Directories/Downloads");
    for (int i = 0; i < downloadsSize; ++i) {
        m_settings->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings->value("path").toString());
        dir.separateFolders = m_settings->value("sepFolders", false).toBool();
        dir.autoReload = m_settings->value("autoReload", false).toBool();
        m_downloadDirectories.append(dir);
    }
    m_settings->endArray();

    m_musicDirectories.clear();
    int musicSize = m_settings->beginReadArray("Directories/Music");
    for (int i = 0; i < musicSize; ++i) {
        m_settings->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings->value("path").toString());
        dir.separateFolders = m_settings->value("sepFolders", false).toBool();
        dir.autoReload = m_settings->value("autoReload", false).toBool();
        m_musicDirectories.append(dir);
    }
    m_settings->endArray();
}

void DirectorySettings::saveSettings()
{
    m_settings->beginWriteArray("Directories/Movies");
    for (int i = 0, n = m_movieDirectories.count(); i < n; ++i) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("path", m_movieDirectories.at(i).path);
        m_settings->setValue("sepFolders", m_movieDirectories.at(i).separateFolders);
        m_settings->setValue("autoReload", m_movieDirectories.at(i).autoReload);
    }
    m_settings->endArray();

    m_settings->beginWriteArray("Directories/TvShows");
    for (int i = 0, n = m_tvShowDirectories.count(); i < n; ++i) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("path", m_tvShowDirectories.at(i).path);
        m_settings->setValue("autoReload", m_tvShowDirectories.at(i).autoReload);
    }
    m_settings->endArray();

    m_settings->beginWriteArray("Directories/Concerts");
    for (int i = 0, n = m_concertDirectories.count(); i < n; ++i) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("path", m_concertDirectories.at(i).path);
        m_settings->setValue("sepFolders", m_concertDirectories.at(i).separateFolders);
        m_settings->setValue("autoReload", m_concertDirectories.at(i).autoReload);
    }
    m_settings->endArray();

    m_settings->beginWriteArray("Directories/Downloads");
    for (int i = 0, n = m_downloadDirectories.count(); i < n; ++i) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("path", m_downloadDirectories.at(i).path);
        m_settings->setValue("sepFolders", m_downloadDirectories.at(i).separateFolders);
        m_settings->setValue("autoReload", m_downloadDirectories.at(i).autoReload);
    }
    m_settings->endArray();

    m_settings->beginWriteArray("Directories/Music");
    for (int i = 0, n = m_musicDirectories.count(); i < n; ++i) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("path", m_musicDirectories.at(i).path);
        m_settings->setValue("sepFolders", m_musicDirectories.at(i).separateFolders);
        m_settings->setValue("autoReload", m_musicDirectories.at(i).autoReload);
    }
    m_settings->endArray();
}

const QVector<SettingsDir> &DirectorySettings::movieDirectories() const
{
    return m_movieDirectories;
}

const QVector<SettingsDir> &DirectorySettings::tvShowDirectories() const
{
    return m_tvShowDirectories;
}

const QVector<SettingsDir> &DirectorySettings::concertDirectories() const
{
    return m_concertDirectories;
}

const QVector<SettingsDir> &DirectorySettings::musicDirectories() const
{
    return m_musicDirectories;
}

const QVector<SettingsDir> &DirectorySettings::downloadDirectories() const
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
