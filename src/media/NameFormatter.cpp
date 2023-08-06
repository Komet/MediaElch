#include "media/NameFormatter.h"

#include "log/Log.h"
#include "utils/Meta.h"

#include <QReadLocker>
#include <QRegularExpression>
#include <QStringList>
#include <QWriteLocker>
#include <memory>
#include <utility>

NameFormatter& NameFormatter::instance()
{
    static NameFormatter s_formatter;
    return s_formatter;
}

void NameFormatter::setExcludeWords(QStringList excludeWords)
{
    std::sort(excludeWords.begin(), excludeWords.end(), NameFormatter::lengthLessThan);

    {
        QReadLocker readLock(&instance().m_lock);
        if (instance().m_allExcludeWords == excludeWords) {
            // we have the same words, no need to reconfigure everything.
            return;
        }
    }

    QStringList excludeWordsNoRegEx;
    QVector<QRegularExpression> excludeWordsRegEx;

    const QRegularExpression specialCharacterReEx(
        "[$&+\\[\\],:;=?@#|'<>^*()%!]", QRegularExpression::CaseInsensitiveOption);

    Q_ASSERT(specialCharacterReEx.isValid());

    for (const QString& word : asConst(excludeWords)) {
        if (specialCharacterReEx.match(word).hasMatch()) {
            excludeWordsNoRegEx << word;
            continue;
        }

        QString sanitized = word;
        sanitized.replace("-", "[-]");
        sanitized.replace(".", "[.]");

        QRegularExpression wordRegEx(QStringLiteral(R"((?:^|[-_(\s.[,]+)%1(?:[-_\s.)\],]+|$))").arg(sanitized),
            QRegularExpression::CaseInsensitiveOption);

        if (wordRegEx.isValid()) {
            excludeWordsRegEx.push_back(std::move(wordRegEx));

        } else {
            qCDebug(generic) << "[NameFormatter] Couldn't use exclude word (invalid RegEx):" << word << ";"
                             << sanitized;
        }
    }

    {
        QWriteLocker writeLock(&instance().m_lock);
        instance().m_allExcludeWords = excludeWords;
        instance().m_excludeWordsNoRegEx = excludeWordsNoRegEx;
        instance().m_excludeWordsRegEx = excludeWordsRegEx;
    }
}

QString NameFormatter::excludeWords(QString name)
{
    // Copy due to possibility that multithreaded access modifies the array.
    QReadLocker readLock(&instance().m_lock);
    const QVector<QRegularExpression> wordsRegEx = instance().m_excludeWordsRegEx;
    const QStringList wordsNoRegEx = instance().m_excludeWordsNoRegEx;
    readLock.unlock();

    for (const QString& word : wordsNoRegEx) {
        name.replace(word, "", Qt::CaseInsensitive);
    }

    QRegularExpressionMatch match;
    for (const QRegularExpression& word : wordsRegEx) {
        match = word.match(name);
        auto pos = match.capturedStart();
        while (pos >= 0) {
            name = name.remove(pos, match.captured(0).length());
            name = name.insert(pos, ' ');
            match = word.match(name);
            pos = match.capturedStart();
        }
    }

    name.replace(QRegularExpression("[.][.]+"), ".");
    name.replace(QRegularExpression("[-][-]+"), "-");

    // remove "- _" at the end of a name
    QRegularExpression delimiterRegEx("[-\\s_]+$");
    name.remove(delimiterRegEx);

    // remove spaces at the start and end which may have been introduced
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
    auto pos = match.capturedStart();
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
    auto pos = name.lastIndexOf(rx);
    name = name.left(pos);
    return name;
}

bool NameFormatter::lengthLessThan(const QString& s1, const QString& s2)
{
    return s1.length() > s2.length();
}
