#include "data/Locale.h"

#include <QDebug>

namespace mediaelch {

Locale Locale::English = Locale("en-US");
Locale Locale::NoLocale = Locale("xx-XX");

Locale::Locale() : m_lang("en"), m_country("US")
{
}

Locale::Locale(const QString& locale)
{
    QStringList split = locale.split('-');
    m_lang = split.first();
    if (split.length() > 1) {
        m_country = split[1];
    }
    if (split.size() > 2) {
        qWarning() << "[Locale] Invalid locale format:" << locale;
    }
}

QString Locale::languageTranslated() const
{
    static QMap<QString, QString> localeTextMap = {{"ar", tr("Arabic")},
        {"ar-AE", tr("Arabic (U.A.E.)")},
        {"ar-SA", tr("Arabic (Saudi Arabia)")},
        {"be-BY", tr("Belarusian")},
        {"bg", tr("Bulgarian")},
        {"bg-BG", tr("Bulgarian")},
        {"bn-BD", tr("Bengali")},
        {"ca-ES", tr("Catalan (Spain)")},
        {"ch-GU", tr("Chamorro (Guam)")},
        {"cn-CN", tr("Cantonese")},
        {"cs", tr("Czech")},
        {"cs-CZ", tr("Czech")},
        {"da", tr("Danish")},
        {"da-DK", tr("Danish")},
        {"de", tr("German")},
        {"de-AT", tr("German (AT)")},
        {"de-CH", tr("German (CH)")},
        {"de-DE", tr("German (DE)")},
        {"el", tr("Greek")},
        {"el-GR", tr("Greek")},
        {"en", tr("English")},
        {"en-AU", tr("English (Australia)")},
        {"en-CA", tr("English (Canada)")},
        {"en-GB", tr("English (GB)")},
        {"en-NZ", tr("English (NZ)")},
        {"en-US", tr("English (US)")},
        {"eo-EO", tr("Esperanto")},
        {"es", tr("Spanish")},
        {"es-ES", tr("Spanish")},
        {"es-MX", tr("Spanish (Mexico)")},
        {"et-EE", tr("Estonian")},
        {"eu-ES", tr("Basque (Spain)")},
        {"fa-IR", tr("Persian (Iran)")},
        {"fi", tr("Finnish")},
        {"fi-FI", tr("Finnish")},
        {"fr", tr("French")},
        {"fr-CA", tr("French (Canada)")},
        {"fr-FR", tr("French")},
        {"gl-ES", tr("Galician (Spain)")},
        {"he", tr("Hebrew")},
        {"he-IL", tr("Hebrew (Israel)")},
        {"hi-IN", tr("Hindi (India)")},
        {"hr", tr("Croatian")},
        {"hr-HR", tr("Croatian (Croatia)")},
        {"hu", tr("Hungarian")},
        {"hu-HU", tr("Hungarian")},
        {"id-ID", tr("Indonesian")},
        {"it", tr("Italian")},
        {"it-IT", tr("Italian")},
        {"ja", tr("Japanese")},
        {"ja-JP", tr("Japanese")},
        {"ka-GE", tr("Georgian")},
        {"kk-KZ", tr("Kazakh")},
        {"kn-IN", tr("Kannada")},
        {"ko", tr("Korean")},
        {"ko-KR", tr("Korean")},
        {"lt-LT", tr("Lithuanian")},
        {"lv-LV", tr("Latvian")},
        {"ml-IN", tr("Malayalam")},
        {"ms-MY", tr("Malay (Malaysia)")},
        {"ms-SG", tr("Malay (Singapore)")},
        {"nb-NO", tr("Norwegian Bokmål")},
        {"nl", tr("Dutch")},
        {"nl-NL", tr("Dutch")},
        {"no", tr("Norwegian")},
        {"no-NO", tr("Norwegian")},
        {"pl", tr("Polish")},
        {"pl-PL", tr("Polish")},
        {"pt", tr("Portuguese")},
        {"pt-BR", tr("Portuguese (Brazil)")},
        {"pt-PT", tr("Portuguese (Portugal)")},
        {"ro-RO", tr("Romanian")},
        {"ru", tr("Russian")},
        {"ru-RU", tr("Russian")},
        {"si-LK", tr("Sinhalese (Sri Lanka)")},
        {"sk-SK", tr("Slovak")},
        {"sl", tr("Slovene")},
        {"sl-SI", tr("Slovenian")},
        {"sq-AL", tr("Albanian")},
        {"sr-RS", tr("Serbian")},
        {"sv", tr("Swedish")},
        {"sv-SE", tr("Swedish")},
        {"ta-IN", tr("Tamil (India)")},
        {"te-IN", tr("Telugu (India)")},
        {"th-TH", tr("Thai")},
        {"tl-PH", tr("Tagalog (Philippines)")},
        {"tr", tr("Turkish")},
        {"tr-TR", tr("Turkish")},
        {"uk-UA", tr("Ukrainian")},
        {"vi-VN", tr("Vietnamese")},
        {"zh", tr("Chinese")},
        {"zh-CN", tr("Chinese (PRC)")},
        {"zh-HK", tr("Chinese (Hong Kong)")},
        {"zh-TW", tr("Chinese (Taiwan)")},
        {"zu-ZA", tr("Zulu")},
        // special case, see Locale::NoLocale
        {"xx-XX", tr("No language available")}};

    QString locale = toString();

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

std::ostream& operator<<(std::ostream& os, const Locale& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const Locale& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Locale(" << id.toString() << ')';
    return debug;
}

bool operator==(const Locale& lhs, const Locale& rhs)
{
    return lhs.toString() == rhs.toString();
}

bool operator!=(const Locale& lhs, const Locale& rhs)
{
    return !(lhs == rhs);
}

// no-op
} // namespace mediaelch
