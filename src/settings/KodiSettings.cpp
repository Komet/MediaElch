#include "settings/KodiSettings.h"

void KodiSettings::loadSettings()
{
    // XBMC
    m_xbmcHost = m_settings->value("XBMC/RemoteHost").toString();
    m_xbmcPort = m_settings->value("XBMC/RemotePort", 80).toInt();
    m_xbmcUser = m_settings->value("XBMC/RemoteUser").toString();
    m_xbmcPassword = m_settings->value("XBMC/RemotePassword").toString();
}

void KodiSettings::saveSettings()
{
    // XBMC
    m_settings->setValue("XBMC/RemoteHost", m_xbmcHost);
    m_settings->setValue("XBMC/RemotePort", m_xbmcPort);
    m_settings->setValue("XBMC/RemoteUser", m_xbmcUser);
    m_settings->setValue("XBMC/RemotePassword", m_xbmcPassword);
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

void KodiSettings::setXbmcHost(QString host)
{
    m_xbmcHost = host;
}

void KodiSettings::setXbmcPort(int port)
{
    m_xbmcPort = port;
}
