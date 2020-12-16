#include "file/NameFormatter.h"

#include <QRegExp>
#include <QStringList>

NameFormatter::NameFormatter(QStringList excludeWords, QObject* parent) : QObject(parent), m_excludedWords{excludeWords}
{
    std::sort(m_excludedWords.begin(), m_excludedWords.end(), NameFormatter::lengthLessThan);
}

QString NameFormatter::excludeWords(QString name)
{
    const QStringList braces = {".", "(", ")", "[", "]", "<", ">"};
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    for (const QString& word : m_excludedWords) {
        if (braces.contains(word)) {
            // Check if the word is a brace...
            name.replace(word, "");
            continue;
        }
        // ...or ignore words with special characters... (TODO: may not be safe)
        rx.setPattern("[$&+,:;=?@#|'<>.^*()%!-]");
        if (rx.indexIn(word) > -1) {
            continue;
        }
        // ...otherwise who knows how this regex would look like
        rx.setPattern(R"((^|[-_(\s.[,]+))" + word + R"(([-_\s.)\],]+|$))");
        if (!rx.isValid()) {
            continue;
        }
        int pos = rx.indexIn(name);
        while (pos >= 0) {
            name = name.remove(pos, rx.cap(0).length());
            name = name.insert(pos, ' ');
            pos = rx.indexIn(name);
        }
    }

    // remove "- _" at the end of a name
    rx.setPattern(R"re([-\s_])re");
    while (name.length() > 0 && rx.lastIndexIn(name) == name.length() - 1) {
        name.chop(1);
    }
    // remove spaces at the start end end which may have been introduced
    return name.trimmed();
}

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

QString NameFormatter::removeParts(QString name)
{
    QRegExp rx("([\\-\\s\\(\\)\\._]+((a|b|c|d|e|f)|((part|cd|xvid)"
               "[\\-\\s\\._]*\\d+))[\\-_\\s\\.\\(\\)]*)",
        Qt::CaseInsensitive);
    int pos = rx.lastIndexIn(name);
    name = name.left(pos);
    return name;
}

bool NameFormatter::lengthLessThan(const QString& s1, const QString& s2)
{
    return s1.length() > s2.length();
}
