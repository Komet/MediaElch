#pragma once

#include <QSettings>
#include <QString>

class NetworkSettings
{
public:
    void loadSettings();
    void saveSettings();
    void setQSettings(QSettings* settings) { m_settings = settings; }

    bool useProxy() const;
    int proxyType() const;
    QString proxyHost() const;
    int proxyPort() const;
    QString proxyUsername() const;
    QString proxyPassword() const;

    void setUseProxy(bool use);
    void setProxyType(int type);
    void setProxyHost(QString host);
    void setProxyPort(uint16_t port);
    void setProxyUsername(QString username);
    void setProxyPassword(QString password);

private:
    void setupProxy();

    QSettings* m_settings = nullptr;

    bool m_useProxy = false;
    int m_proxyType = 0;
    QString m_proxyHost;
    uint16_t m_proxyPort = 0;
    QString m_proxyUsername;
    QString m_proxyPassword;
};
