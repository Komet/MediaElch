#pragma once

#include <QCoreApplication>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

namespace mediaelch {
namespace scraper {

class Locale
{
    Q_DECLARE_TR_FUNCTIONS(Locale)

public:
    Locale(const QString& _locale) : m_locale(_locale) {}

    Locale(QString language, QString country) : m_locale{QStringLiteral("%1-%2").arg(language, country)} {}

    QString language() const { return m_locale.split("-")[0]; }
    QString country() const { return m_locale.split("-")[1]; }
    QString toString(char delimiter = '-') const { return QString(m_locale).replace('-', delimiter); }

    QString languageTranslated() const
    {
        static QMap<QString, QString> localeTextMap = {{"ar", tr("Arabic")},
            {"bg", tr("Bulgarian")},
            {"zh-TW", tr("Chinese (T)")},
            {"zh-CN", tr("Chinese (S)")},
            {"hr", tr("Croatian")},
            {"cs", tr("Czech")},
            {"da", tr("Danish")},
            {"nl", tr("Dutch")},
            {"en", tr("English")},
            {"en-GB", tr("English (GB)")},
            {"en-US", tr("English (US)")},
            {"fi", tr("Finnish")},
            {"fr", tr("French")},
            {"fr-CA", tr("French (Canada)")},
            {"de", tr("German")},
            {"de-AT", tr("German (AT)")},
            {"de-CH", tr("German (CH)")},
            {"de-DE", tr("German (DE)")},
            {"el", tr("Greek")},
            {"he", tr("Hebrew")},
            {"hu", tr("Hungarian")},
            {"it", tr("Italian")},
            {"ja", tr("Japanese")},
            {"ko", tr("Korean")},
            {"no", tr("Norwegian")},
            {"pl", tr("Polish")},
            {"pt-BR", tr("Portuguese (Brazil)")},
            {"pt-PT", tr("Portuguese (Portugal)")},
            {"ru", tr("Russian")},
            {"sl", tr("Slovene")},
            {"es", tr("Spanish")},
            {"es-MX", tr("Spanish (Mexico)")},
            {"sv", tr("Swedish")},
            {"tr", tr("Turkish")}};
        if (!localeTextMap.contains(m_locale)) {
            return tr("Unknown language: %1").arg(m_locale);
        }
        return localeTextMap[m_locale];
    }

private:
    QString m_locale;
};
} // namespace scraper

} // namespace mediaelch
