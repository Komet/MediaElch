#pragma once

#include <QObject>
#include <QReadWriteLock>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

/// \brief Name formatter for movies/TV shows and other files.
/// \details Singleton that provides thread-safe mechanism for "prettifying" names.
/// \note  There is only one instance of this class (singleton).  While I
///        mostly dislike singletons, in this case there are reasons.  If we
///        do not use a singleton and do not store QRegularExpression objects,
///        each movie that is loaded has to instantiate all QRegularExpressions
///        for the "exclude word list".  Due to JIT compilation, this has
///        a _huge_ time disadvantage.  In some tests, 50% of the time went
///        into the NameFormatter prior to using a singleton.
///        See https://github.com/Komet/MediaElch/pull/1229 for more details.
class NameFormatter
{
public:
    static NameFormatter& instance();

    static void setExcludeWords(QStringList excludeWords);

    /// \brief Returns a new string with excluded words removed.
    /// \param name name to remove excluded words from
    static QString excludeWords(QString name);

    /// \brief Removes exclude dwords, changes "." and "_" to " " and removes all " - " at the end of the name.
    static QString formatName(QString name, bool replaceDots = true, bool replaceUnderscores = true);

    /// \brief Removes the last part of a name, looking like " - cd1" or "_a"
    static QString removeParts(QString name);

private:
    explicit NameFormatter() = default;

    static bool lengthLessThan(const QString& s1, const QString& s2);

private:
    QStringList m_allExcludeWords;
    /// \brief Excluded words that contain special characters not suitable for regular expressions.
    QStringList m_excludeWordsNoRegEx;
    QVector<QRegularExpression> m_excludeWordsRegEx;
    QReadWriteLock m_lock;
};
