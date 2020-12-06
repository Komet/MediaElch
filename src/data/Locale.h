#pragma once

#include <QCoreApplication>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <ostream>

namespace mediaelch {

class Locale
{
    Q_DECLARE_TR_FUNCTIONS(Locale)

public:
    /// \brief Locale for en-US. Can be used as default.
    static Locale English;
    /// \brief Locale often used as sentinel.
    static Locale NoLocale;

public:
    /// \brief Default constructor required for Qt containers like QVector.
    ///        Defaults to en-US.
    Locale();
    /*implicit*/ Locale(const char* locale) : Locale(QString(locale)) {}
    /*implicit*/ Locale(const QString& locale);

    Locale(QString language, QString country) : m_lang{language}, m_country{country} {}

    const QString& language() const { return m_lang; }
    const QString& country() const { return m_country; }
    QString toString(char delimiter = '-') const { return hasCountry() ? m_lang + delimiter + m_country : m_lang; }

    bool hasCountry() const { return !m_country.isEmpty(); }

    /// \brief Return a human readable, translated language name for the locale.
    /// \details If the locale, e.g. en-US, is not translated, it is checked whether "en" is available.
    ///          If so, then the country is appended in parentheses in a country is available.
    ///          If the language is still not found then simply the locale in string representation is returned.
    QString languageTranslated() const;

private:
    QString m_lang;
    QString m_country;
};

bool operator==(const Locale& lhs, const Locale& rhs);
bool operator!=(const Locale& lhs, const Locale& rhs);

std::ostream& operator<<(std::ostream& os, const Locale& id);
QDebug operator<<(QDebug debug, const Locale& id);

} // namespace mediaelch
