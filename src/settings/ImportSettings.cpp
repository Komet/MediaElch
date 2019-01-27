#include "settings/ImportSettings.h"

void ImportSettings::loadSettings()
{
    m_unrar = m_settings->value("Downloads/Unrar").toString();
    m_makeMkvCon = m_settings->value("Downloads/MakeMkvCon").toString();
}

void ImportSettings::saveSettings()
{
    m_settings->setValue("Downloads/Unrar", m_unrar);
    m_settings->setValue("Downloads/MakeMkvCon", m_makeMkvCon);
}

QString ImportSettings::makeMkvCon() const
{
    return m_makeMkvCon;
}

QString ImportSettings::unrar() const
{
    return m_unrar;
}

void ImportSettings::setUnrar(QString unrar)
{
    m_unrar = unrar;
}

void ImportSettings::setMakeMkvCon(QString makeMkvCon)
{
    m_makeMkvCon = makeMkvCon;
}
