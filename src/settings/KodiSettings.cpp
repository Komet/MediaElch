#include "settings/KodiSettings.h"

#include "log/Log.h"

namespace {

static constexpr char moduleName[] = "import";
static const Settings::Key KEY_SETTINGS_KODI_REMOTE_HOST(moduleName, "XBMC/RemoteHost");
static const Settings::Key KEY_SETTINGS_KODI_REMOTE_PORT(moduleName, "XBMC/RemotePort");
static const Settings::Key KEY_SETTINGS_KODI_REMOTE_USER(moduleName, "XBMC/RemoteUser");
static const Settings::Key KEY_SETTINGS_KODI_REMOTE_PASSWORD(moduleName, "XBMC/RemotePassword");
static const Settings::Key KEY_SETTINGS_KODI_VERSION(moduleName, "kodi/version");

} // namespace

namespace mediaelch {

KodiSettings::KodiSettings(Settings& settings, QObject* parent) : QObject(parent), m_settings{settings}
{
    settings.onSettingChanged(KEY_SETTINGS_KODI_REMOTE_HOST, this, [this]() { emit xbmcHostChanged(xbmcHost()); });
    settings.onSettingChanged(KEY_SETTINGS_KODI_REMOTE_PORT, this, [this]() { emit xbmcPortChanged(xbmcPort()); });
    settings.onSettingChanged(KEY_SETTINGS_KODI_REMOTE_USER, this, [this]() { emit xbmcUserChanged(xbmcUser()); });
    settings.onSettingChanged(
        KEY_SETTINGS_KODI_REMOTE_PASSWORD, this, [this]() { emit xbmcPasswordChanged(xbmcPassword()); });
    settings.onSettingChanged(KEY_SETTINGS_KODI_VERSION, this, [this]() { emit kodiVersionChanged(kodiVersion()); });
}

void KodiSettings::init()
{
    m_settings.setDefaultValue(KEY_SETTINGS_KODI_REMOTE_HOST, "127.0.0.1");
    m_settings.setDefaultValue(KEY_SETTINGS_KODI_REMOTE_PORT, QVariant::fromValue(80));
    m_settings.setDefaultValue(KEY_SETTINGS_KODI_REMOTE_USER, "kodi");
    m_settings.setDefaultValue(KEY_SETTINGS_KODI_REMOTE_PASSWORD, "kodi");
    m_settings.setDefaultValue(
        KEY_SETTINGS_KODI_VERSION, QVariant::fromValue(mediaelch::KodiVersion::latest().toInt()));
}

void KodiSettings::setXbmcUser(QString xbmcUser)
{
    m_settings.setValue(KEY_SETTINGS_KODI_REMOTE_USER, xbmcUser);
}

QString KodiSettings::xbmcUser()
{
    return m_settings.value(KEY_SETTINGS_KODI_REMOTE_USER).toString();
}

QString KodiSettings::xbmcPassword()
{
    return m_settings.value(KEY_SETTINGS_KODI_REMOTE_PASSWORD).toString();
}

void KodiSettings::setXbmcPassword(QString password)
{
    m_settings.setValue(KEY_SETTINGS_KODI_REMOTE_PASSWORD, password);
}

mediaelch::KodiVersion KodiSettings::kodiVersion()
{
    bool ok = false;
    const int version = m_settings.value(KEY_SETTINGS_KODI_VERSION).toInt(&ok);
    if (!ok || version == 0 || !mediaelch::KodiVersion::isValid(version)) {
        setKodiVersion(mediaelch::KodiVersion::latest());
        return mediaelch::KodiVersion::latest();
    }
    return mediaelch::KodiVersion(version);
}

void KodiSettings::setKodiVersion(mediaelch::KodiVersion version)
{
    m_settings.setValue(KEY_SETTINGS_KODI_VERSION, version.toInt());
}

QString KodiSettings::xbmcHost()
{
    return m_settings.value(KEY_SETTINGS_KODI_REMOTE_HOST).toString();
}

void KodiSettings::setXbmcHost(QString host)
{
    m_settings.setValue(KEY_SETTINGS_KODI_REMOTE_HOST, host);
}

int KodiSettings::xbmcPort()
{
    int xbmcPort = m_settings.value(KEY_SETTINGS_KODI_REMOTE_PORT).toInt();
    if (xbmcPort > (2 << 16) || xbmcPort < 1) {
        setXbmcPort(80);
        return 80;
    }
    return xbmcPort;
}

void KodiSettings::setXbmcPort(int port)
{
    m_settings.setValue(KEY_SETTINGS_KODI_REMOTE_PORT, port);
}

} // namespace mediaelch
