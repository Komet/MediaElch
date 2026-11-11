#include "settings/KodiXmlSettings.h"

#include "settings/Settings.h"

namespace {

static const Settings::Key KEY_SETTINGS_KODI_VERSION("import", "kodi/version");

} // namespace

namespace mediaelch {

KodiXmlSettings::KodiXmlSettings(Settings& settings) : m_settings{settings}
{
}

void KodiXmlSettings::init()
{
    m_settings.setDefaultValue(KEY_SETTINGS_KODI_VERSION, QVariant::fromValue(KodiVersion::latest().toInt()));
}

KodiVersion KodiXmlSettings::kodiVersion()
{
    bool ok = false;
    const int version = m_settings.value(KEY_SETTINGS_KODI_VERSION).toInt(&ok);
    if (!ok || !KodiVersion::isValid(version)) {
        m_settings.setValue(KEY_SETTINGS_KODI_VERSION, KodiVersion::latest().toInt());
        return KodiVersion::latest();
    }
    return KodiVersion(version);
}

bool KodiXmlSettings::writeThumbUrlsToNfo()
{
    return m_settings.advanced()->writeThumbUrlsToNfo();
}

bool KodiXmlSettings::useFirstStudioOnly()
{
    return m_settings.advanced()->useFirstStudioOnly();
}

bool KodiXmlSettings::ignoreDuplicateOriginalTitle()
{
    return m_settings.ignoreDuplicateOriginalTitle();
}

bool KodiXmlSettings::usePlotForOutline()
{
    return m_settings.usePlotForOutline();
}

QVector<DataFile> KodiXmlSettings::dataFiles(DataFileType type)
{
    return m_settings.dataFiles(type);
}

QVector<DataFile> KodiXmlSettings::dataFiles(ImageType type)
{
    return m_settings.dataFiles(type);
}

MovieSetArtworkType KodiXmlSettings::movieSetArtworkType()
{
    return m_settings.movieSetArtworkType();
}

mediaelch::DirectoryPath KodiXmlSettings::movieSetArtworkDirectory()
{
    return m_settings.movieSetArtworkDirectory();
}

} // namespace mediaelch
