#include "settings/NetworkSettings.h"

#include <QNetworkProxy>

void NetworkSettings::loadSettings()
{
    // Proxy
    m_useProxy = m_settings->value("Proxy/Enable", false).toBool();
    m_proxyType = m_settings->value("Proxy/Type", 0).toInt();
    m_proxyHost = m_settings->value("Proxy/Host").toString();
    m_proxyPort = static_cast<uint16_t>(m_settings->value("Proxy/Port", 0).toInt());
    m_proxyUsername = m_settings->value("Proxy/Username").toString();
    m_proxyPassword = m_settings->value("Proxy/Password").toString();
    setupProxy();
}

void NetworkSettings::saveSettings()
{
    // Proxy
    m_settings->setValue("Proxy/Enable", m_useProxy);
    m_settings->setValue("Proxy/Type", m_proxyType);
    m_settings->setValue("Proxy/Host", m_proxyHost);
    m_settings->setValue("Proxy/Port", m_proxyPort);
    m_settings->setValue("Proxy/Username", m_proxyUsername);
    m_settings->setValue("Proxy/Password", m_proxyPassword);
    setupProxy();
}

/**
 * @brief Holds if a proxy should be used
 * @return Proxy enabled
 */
bool NetworkSettings::useProxy() const
{
    return m_useProxy;
}

/**
 * @brief Holds the type of the proxy (0 HTTP, 1 SOCKS5)
 * @return Proxy type
 */
int NetworkSettings::proxyType() const
{
    return m_proxyType;
}

/**
 * @brief Holds the host of the proxy
 * @return Proxy host
 */
QString NetworkSettings::proxyHost() const
{
    return m_proxyHost;
}

/**
 * @brief Holds the port of the proxy
 * @return Proxy port
 */
int NetworkSettings::proxyPort() const
{
    return m_proxyPort;
}

/**
 * @brief Holds the username of the proxy
 * @return Proxy username
 */
QString NetworkSettings::proxyUsername() const
{
    return m_proxyUsername;
}

/**
 * @brief Holds the password of the proxy
 * @return Proxy password
 */
QString NetworkSettings::proxyPassword() const
{
    return m_proxyPassword;
}

/**
 * @brief Sets if a proxy should be used
 * @param use Enable proxy
 */
void NetworkSettings::setUseProxy(bool use)
{
    m_useProxy = use;
}

/**
 * @brief Sets the proxy type
 * @param type 0 HTTP, 1 SOCKS5
 */
void NetworkSettings::setProxyType(int type)
{
    m_proxyType = type;
}

/**
 * @brief Sets the host of the proxy
 * @param host Proxy host
 */
void NetworkSettings::setProxyHost(QString host)
{
    m_proxyHost = host;
}

/**
 * @brief Sets the port of the proxy
 * @param port Proxy port
 */
void NetworkSettings::setProxyPort(uint16_t port)
{
    m_proxyPort = port;
}

/**
 * @brief Sets the username to use when connecting to the proxy
 * @param username Proxy username
 */
void NetworkSettings::setProxyUsername(QString username)
{
    m_proxyUsername = username;
}

/**
 * @brief Sets the password to use when connecting to the proxy
 * @param password Proxy password
 */
void NetworkSettings::setProxyPassword(QString password)
{
    m_proxyPassword = password;
}

/**
 * @brief Sets up the proxy
 */
void NetworkSettings::setupProxy()
{
    QNetworkProxy proxy;
    if (!m_useProxy) {
        proxy.setType(QNetworkProxy::NoProxy);
    } else if (m_proxyType == 0) {
        proxy.setType(QNetworkProxy::HttpProxy);
    } else {
        proxy.setType(QNetworkProxy::Socks5Proxy);
    }
    proxy.setHostName(m_proxyHost);
    proxy.setPort(m_proxyPort);
    proxy.setUser(m_proxyUsername);
    proxy.setPassword(m_proxyPassword);
    QNetworkProxy::setApplicationProxy(proxy);
}
