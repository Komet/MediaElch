#include "settings/KodiSettings.h"

#include <QDebug>

using namespace mediaelch;

void KodiSettings::loadSettings()
{
    m_xbmcHost = m_settings->value("XBMC/RemoteHost").toString();
    m_xbmcPort = m_settings->value("XBMC/RemotePort", 80).toInt();
    if (m_xbmcPort > (2 << 16) || m_xbmcPort < 1) {
        m_xbmcPort = 80;
    }
    m_xbmcUser = m_settings->value("XBMC/RemoteUser").toString();
    m_xbmcPassword = m_settings->value("XBMC/RemotePassword").toString();

    const int version = m_settings->value("kodi/version").toInt();
    if (!KodiVersion::isValid(version)) {
        qWarning() << "Found invalid Kodi version" << version
                   << "in settings; default is:" << KodiVersion::latest().toInt();
        setKodiVersion(KodiVersion::latest());
    } else {
        setKodiVersion(KodiVersion(version));
    }
}

void KodiSettings::saveSettings()
{
    m_settings->setValue("XBMC/RemoteHost", m_xbmcHost);
    m_settings->setValue("XBMC/RemotePort", m_xbmcPort);
    m_settings->setValue("XBMC/RemoteUser", m_xbmcUser);
    m_settings->setValue("XBMC/RemotePassword", m_xbmcPassword);
    m_settings->setValue("kodi/version", m_version.toInt());
}

void KodiSettings::setXbmcUser(QString user)
{
    m_xbmcUser = user;
}

QString KodiSettings::xbmcUser() const
{
    return m_xbmcUser;
}

QString KodiSettings::xbmcPassword() const
{
    return m_xbmcPassword;
}

const KodiVersion& KodiSettings::kodiVersion() const
{
    return m_version;
}

QString KodiSettings::xbmcHost() const
{
    return m_xbmcHost;
}

int KodiSettings::xbmcPort() const
{
    return m_xbmcPort;
}

void KodiSettings::setXbmcPassword(QString password)
{
    m_xbmcPassword = password;
}

void KodiSettings::setKodiVersion(KodiVersion version)
{
    m_version = version;
}

void KodiSettings::setXbmcHost(QString host)
{
    m_xbmcHost = host;
}

void KodiSettings::setXbmcPort(int port)
{
    m_xbmcPort = port;
}
