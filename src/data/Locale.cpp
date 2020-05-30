#include "data/Locale.h"

#include <QDebug>

namespace mediaelch {

Locale Locale::English = Locale("en-US");

QString Locale::languageTranslated() const
{
    static QMap<QString, QString> localeTextMap = {{"ar", tr("Arabic")},
        {"bg", tr("Bulgarian")},
        {"zh", tr("Chinese")},
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
        {"pt", tr("Portuguese")},
        {"pt-BR", tr("Portuguese (Brazil)")},
        {"pt-PT", tr("Portuguese (Portugal)")},
        {"ru", tr("Russian")},
        {"sl", tr("Slovene")},
        {"es", tr("Spanish")},
        {"es-MX", tr("Spanish (Mexico)")},
        {"sv", tr("Swedish")},
        {"tr", tr("Turkish")}};

    const QString locale = toString();

    if (localeTextMap.contains(locale)) {
        return localeTextMap[locale];
    }
    if (localeTextMap.contains(m_lang)) {
        if (hasCountry()) {
            return QStringLiteral("%1 (%2)").arg(localeTextMap[language()], m_country);
        }
        return localeTextMap[m_lang];
    }
    qDebug() << "[Locale] Missing name for" << locale;
    return locale;
}

// no-op
} // namespace mediaelch
