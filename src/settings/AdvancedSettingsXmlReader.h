#pragma once

#include "settings/AdvancedSettings.h"

#include <QXmlStreamReader>

class AdvancedSettingsXmlReader
{
public:
    enum class ParseErrorType
    {
        FileNotFound,
        FileIsReadOnly,
        NoMainTag,
        UnsupportedTag,
        InvalidValue,
        InvalidAttributeValue
    };

    struct ParseError
    {
        ParseErrorType type;
        QString tag;
    };

    using ValidationMessages = QVector<ParseError>;
    static QPair<AdvancedSettings, ValidationMessages> loadFromDefaultPath();
    static QPair<AdvancedSettings, ValidationMessages> loadFromXml(QString xml);

    /// Translated ParseError messages
    static const QMap<ParseErrorType, QString> errors;

private:
    AdvancedSettingsXmlReader() = default;

    /// @return The path to the advancedsettings.xml file. Checks two possible locations.
    QString getFilePath();
    QByteArray getAdvancedSettingsXml(const QString& filepath);
    void parseSettings(const QString& xmlSource);

    void loadLog();
    void loadGui();
    void loadSortTokens();
    void loadFilters();
    void loadMappings(QHash<QString, QString>& map);
    void loadExcludePatterns();

    void addError(QString tag, ParseErrorType type);
    void addWarning(QString tag, ParseErrorType type);
    void skipUnsupportedTag();
    void invalidValue();
    QString currentLocation() const;

    void expectBool(bool& valueToSet);
    void expectInt(int& valueToSet);
    /// Expects the contents of the current xml tag to be an integer
    /// and the callback with the integer to return true.
    template<typename Callback>
    void expectIntChecked(int& valueToSet, Callback fct);

private:
    AdvancedSettings m_settings;
    ValidationMessages m_messages;
    // settings file, set in parseSettings()
    QXmlStreamReader m_xml;
};
