#include "settings/ImportSettings.h"


namespace {

constexpr char moduleName[] = "import";
const Settings::Key KEY_SETTINGS_DOWNLOAD_UNRAR(moduleName, "Downloads/Unrar");
const Settings::Key KEY_SETTINGS_DOWNLOAD_MAKE_MKV(moduleName, "Downloads/MakeMkvCon");
const Settings::Key KEY_DOWNLOADS_DELETE_ARCHIVES(moduleName, "Downloads/DeleteArchives");

} // namespace

ImportSettings::ImportSettings(Settings& settings, QObject* parent) : QObject(parent), m_settings{settings}
{
}

ImportSettings::~ImportSettings() = default;

void ImportSettings::init()
{
    m_settings.setDefaultValue(KEY_SETTINGS_DOWNLOAD_UNRAR, QVariant{});
    m_settings.setDefaultValue(KEY_SETTINGS_DOWNLOAD_MAKE_MKV, QVariant{});
    m_settings.setDefaultValue(KEY_DOWNLOADS_DELETE_ARCHIVES, QVariant::fromValue(false));
}

QString ImportSettings::unrar() const
{
    return m_settings.value(KEY_SETTINGS_DOWNLOAD_UNRAR).toString();
}

QString ImportSettings::makeMkvCon() const
{
    return m_settings.value(KEY_SETTINGS_DOWNLOAD_MAKE_MKV).toString();
}

void ImportSettings::setUnrar(QString unrar)
{
    m_settings.setValue(KEY_SETTINGS_DOWNLOAD_UNRAR, unrar);
}

void ImportSettings::setMakeMkvCon(QString makeMkvCon)
{
    m_settings.setValue(KEY_SETTINGS_DOWNLOAD_MAKE_MKV, makeMkvCon);
}

void ImportSettings::setDeleteArchives(bool deleteArchives)
{
    m_settings.setValue(KEY_DOWNLOADS_DELETE_ARCHIVES, deleteArchives);
}

bool ImportSettings::deleteArchives() const
{
    return m_settings.value(KEY_DOWNLOADS_DELETE_ARCHIVES).toBool();
}
