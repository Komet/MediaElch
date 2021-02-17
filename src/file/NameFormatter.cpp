#include "file/NameFormatter.h"

#include "globals/Meta.h"

#include <QRegularExpression>
#include <QStringList>
#include <utility>

NameFormatter::NameFormatter(QStringList excludeWords, QObject* parent) :
    QObject(parent), m_excludedWords{std::move(excludeWords)}
{
    std::sort(m_excludedWords.begin(), m_excludedWords.end(), NameFormatter::lengthLessThan);
}

QString NameFormatter::excludeWords(QString name)
{
    const QStringList braces = {".", "(", ")", "[", "]", "<", ">"};

    QRegularExpression specialCharacterReEx("[$&+,:;=?@#|'<>.^*()%!-]", QRegularExpression::CaseInsensitiveOption);

    QRegularExpression wordRegEx;
    wordRegEx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;

    for (const QString& word : asConst(m_excludedWords)) {
        if (braces.contains(word)) {
            // Check if the word is a brace...
            name.replace(word, "");
            continue;
        }
        // ...or just replace words with special characters...
        if (specialCharacterReEx.match(word).hasMatch()) {
            name.replace(word, "", Qt::CaseInsensitive);
            continue;
        }
        // ...otherwise who knows how this regex would look like (TODO: may not be safe)
        wordRegEx.setPattern(R"((?:^|[-_(\s.[,]+))" + word + R"((?:[-_\s.)\],]+|$))");
        if (!wordRegEx.isValid()) {
            continue;
        }
        match = wordRegEx.match(name);
        int pos = match.capturedStart();
        while (pos >= 0) {
            name = name.remove(pos, match.captured(0).length());
            name = name.insert(pos, ' ');
            match = wordRegEx.match(name);
            pos = match.capturedStart();
        }
    }

    name.replace(QRegularExpression("[.][.]+"), ".");
    name.replace(QRegularExpression("[-][-]+"), "-");

    // remove "- _" at the end of a name
    QRegularExpression delimiterRegEx("[-\\s_]+$");
    name.remove(delimiterRegEx);

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
    QRegularExpression rx(R"(\([-\s]*\))");
    QRegularExpressionMatch match = rx.match(name);
    int pos = match.capturedStart();
    while (pos >= 0) {
        name = name.remove(pos, match.captured(0).length());
        match = rx.match(name);
        pos = match.capturedStart();
    }

    // remove " - _" at the end of a name
    rx.setPattern("[-\\s_]");
    while (name.length() > 0 && name.lastIndexOf(rx) == name.length() - 1) {
        name.chop(1);
    }
    return name;
}

QString NameFormatter::removeParts(QString name)
{
    QRegularExpression rx(R"re([-_\s().]+([a-f]|(?:(?:part|cd|xvid)[-_\s.]*\d+))[-_\s().]*$)re",
        QRegularExpression::CaseInsensitiveOption);
    int pos = name.lastIndexOf(rx);
    name = name.left(pos);
    return name;
}

bool NameFormatter::lengthLessThan(const QString& s1, const QString& s2)
{
    return s1.length() > s2.length();
}
