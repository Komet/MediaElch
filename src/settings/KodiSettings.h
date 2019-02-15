#pragma once

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

    void setXbmcHost(QString host);
    void setXbmcPort(int port);
    void setXbmcUser(QString user);
    void setXbmcPassword(QString password);

private:
    QSettings* m_settings = nullptr;

    QString m_xbmcHost;
    int m_xbmcPort;
    QString m_xbmcUser;
    QString m_xbmcPassword;
};
