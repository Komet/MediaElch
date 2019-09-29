#include "NameFormatter.h"
#include "settings/Settings.h"

#include <QRegExp>
#include <QStringList>

NameFormatter::NameFormatter(QObject* parent) : QObject(parent)
{
    onUpdateExcludeWords();
    connect(Settings::instance(), &Settings::sigSettingsSaved, this, &NameFormatter::onUpdateExcludeWords);
}

/**
 * @brief Returns an instance of the name formatter
 * @param parent Parent widget
 * @return Instance of name formatter
 */
NameFormatter* NameFormatter::instance(QObject* parent)
{
    static auto* s_formatterInstance = new NameFormatter(parent);
    return s_formatterInstance;
}

/**
 * @brief removes the exclude words, given from settings
 * @param name name to remove ex. words from
 * @return cleaned name
 */
QString NameFormatter::excludeWords(QString name)
{
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    for (const QString& word : m_exWords) {
        rx.setPattern(R"((^|[\(\s\-\.\[]+))" + word + R"(([\s\-\.\)\],]+|$))");
        int pos = rx.indexIn(name);
        while (pos >= 0) {
            name = name.remove(pos, rx.cap(0).length());
            name = name.insert(pos, ' ');
            pos = rx.indexIn(name);
        }

        QStringList braces = QStringList() << "("
                                           << ")"
                                           << "["
                                           << "]";
        if (braces.contains(word)) {
            name.replace(word, "");
        }
    }

    // remove " - _" at the end of a name
    rx.setPattern("[\\-\\s_]");
    while (rx.lastIndexIn(name) == name.length() - 1 && name.length() > 0) {
        name.chop(1);
    }

    return name;
}

/**
 * @brief Removes the exclude words,
 * changes "." and "_" to " "
 * and removes all " - " at the end of the name
 * @param name Not yet formatted name.
 * @return Formatted name
 */
QString NameFormatter::formatName(QString name, bool replaceDots, bool replaceUnderscores)
{
    if (replaceDots) {
        name = name.replace(".", " ");
    }

    if (replaceUnderscores) {
        name = name.replace("_", " ");
    }

    // remove exclude words
    name = excludeWords(name);

    // remove resulting empty brackets
    QRegExp rx(R"(\([\s\-]*\))");
    int pos = rx.indexIn(name);
    while (rx.indexIn(name) >= 0) {
        name = name.remove(pos, rx.cap(0).length());
        pos = rx.indexIn(name);
    }

    // remove " - " at the end of a name
    rx.setPattern("[\\-\\s]");
    while (rx.lastIndexIn(name) == name.length() - 1 && name.length() > 0) {
        name.chop(1);
    }
    return name;
}

/**
 * @brief Removes the last part of a name,
 * looking like " - cd1" or "_a"
 * @param name name
 * @return cleaned name
 */
QString NameFormatter::formatParts(QString name)
{
    QRegExp rx("([\\-\\s\\(\\)\\._]+((a|b|c|d|e|f)|((part|cd|xvid)"
               "[\\-\\s\\._]*\\d+))[\\-_\\s\\.\\(\\)]*)",
        Qt::CaseInsensitive);
    int pos = rx.lastIndexIn(name);
    name = name.left(pos);
    return name;
}

void NameFormatter::onUpdateExcludeWords()
{
    m_exWords = Settings::instance()->excludeWords().remove(" ").split(",", QString::SkipEmptyParts);
    std::sort(m_exWords.begin(), m_exWords.end(), NameFormatter::lengthLessThan);
}

bool NameFormatter::lengthLessThan(const QString& s1, const QString& s2)
{
    return s1.length() > s2.length();
}
