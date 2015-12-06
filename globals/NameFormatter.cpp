#include "NameFormatter.h"
#include "settings/Settings.h"

#include <QRegExp>
#include <QStringList>

NameFormatter *NameFormatter::m_instance = 0;

NameFormatter::NameFormatter(QObject *parent) :
    QObject(parent)
{
    onUpdateExcludeWords();
    connect(Settings::instance(), SIGNAL(sigSettingsSaved()), this, SLOT(onUpdateExcludeWords()));
}

/**
 * @brief Returns an instance of the name formatter
 * @param parent Parent widget
 * @return Instance of name formatter
 */
NameFormatter *NameFormatter::instance(QObject *parent)
{
    if (m_instance == 0) {
        m_instance = new NameFormatter(parent);
    }
    return m_instance;
}

/**
 * @brief removes the exclude words, given from settings
 * @param name name to remove ex. words from
 * @return cleaned name
 */
QString NameFormatter::excludeWords(QString name)
{
    int pos;
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    foreach (const QString &word, m_exWords) {
        pos = 0;
        rx.setPattern("(^|[\\(\\s\\-\\.\\[]+)" + word + "([\\s\\-\\.\\)\\],]+|$)");
        pos = rx.indexIn(name);
        while (pos >= 0) {
            name = name.remove(pos, rx.cap(0).length());
            name = name.insert(pos, ' ');
            pos = rx.indexIn(name);
        }

        QStringList braces = QStringList() << "(" << ")" << "[" << "]";
        if (braces.contains(word))
            name.replace(word, "");
    }

    // remove " - _" at the end of a name
    rx.setPattern("[\\-\\s_]");
    while (rx.lastIndexIn(name) == name.length()-1 && name.length() > 0)
        name.chop(1);

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
    if (replaceDots)
        name = name.replace(".", " ");

    if (replaceUnderscores)
        name = name.replace("_", " ");

    // remove exclude words
    name = excludeWords(name);

    // remove resulting empty brackets
    QRegExp rx("\\([\\s\\-]*\\)");
    int pos = rx.indexIn(name);
    while (rx.indexIn(name) >= 0) {
        name = name.remove(pos, rx.cap(0).length());
        pos = rx.indexIn(name);
    }

    // remove " - " at the end of a name
    rx.setPattern("[\\-\\s]");
    while (rx.lastIndexIn(name) == name.length()-1 && name.length() > 0)
        name.chop(1);
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
    QRegExp rx("([\\-\\s\\(\\)\\._]+((a|b|c|d|e|f)|((part|cd|xvid)" \
               "[\\-\\s\\._]*\\d+))[\\-_\\s\\.\\(\\)]*)",
               Qt::CaseInsensitive);
    int pos = rx.lastIndexIn(name);
    name = name.left(pos);
    return name;
}

void NameFormatter::onUpdateExcludeWords()
{
    m_exWords = Settings::instance()->excludeWords()
            .remove(" ").split(",", QString::SkipEmptyParts);
    qSort(m_exWords.begin(), m_exWords.end(), NameFormatter::lengthLessThan);
}

bool NameFormatter::lengthLessThan(const QString &s1, const QString &s2)
{
    return s1.length() > s2.length();
}
