#pragma once

#include "media_centers/KodiVersion.h"

#include <QSettings>
#include <QString>

class KodiSettings
{
public:
    void loadSettings();
    void saveSettings();
    void setQSettings(QSettings* settings) { m_settings = settings; }

    QString xbmcHost() const;
    int xbmcPort() const;
    QString xbmcUser() const;
    QString xbmcPassword() const;
    const mediaelch::KodiVersion& kodiVersion() const;

    void setXbmcHost(QString host);
    void setXbmcPort(int port);
    void setXbmcUser(QString user);
    void setXbmcPassword(QString password);

    void setKodiVersion(mediaelch::KodiVersion version);

private:
    QSettings* m_settings = nullptr;

    QString m_xbmcHost;
    int m_xbmcPort = 0;
    QString m_xbmcUser;
    QString m_xbmcPassword;
    mediaelch::KodiVersion m_version = mediaelch::KodiVersion::latest();
};
