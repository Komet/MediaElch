#pragma once

#include "globals/MediaDirectory.h"

#include <QSettings>
#include <QVector>

class DirectorySettings
{
public:
    void loadSettings();
    void saveSettings();
    void setQSettings(QSettings* settings) { m_settings = settings; }

    const QVector<mediaelch::MediaDirectory>& movieDirectories() const;
    const QVector<mediaelch::MediaDirectory>& tvShowDirectories() const;
    const QVector<mediaelch::MediaDirectory>& concertDirectories() const;
    const QVector<mediaelch::MediaDirectory>& downloadDirectories() const;
    const QVector<mediaelch::MediaDirectory>& musicDirectories() const;

    void setMovieDirectories(QVector<mediaelch::MediaDirectory> dirs);
    void setTvShowDirectories(QVector<mediaelch::MediaDirectory> dirs);
    void setConcertDirectories(QVector<mediaelch::MediaDirectory> dirs);
    void setDownloadDirectories(QVector<mediaelch::MediaDirectory> dirs);
    void setMusicDirectories(QVector<mediaelch::MediaDirectory> dirs);

private:
    QSettings* m_settings = nullptr;

    QVector<mediaelch::MediaDirectory> m_movieDirectories;
    QVector<mediaelch::MediaDirectory> m_tvShowDirectories;
    QVector<mediaelch::MediaDirectory> m_concertDirectories;
    QVector<mediaelch::MediaDirectory> m_downloadDirectories;
    QVector<mediaelch::MediaDirectory> m_musicDirectories;
};
