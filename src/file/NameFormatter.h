#pragma once

#include <QObject>
#include <QStringList>

class NameFormatter : public QObject
{
    Q_OBJECT
public:
    explicit NameFormatter(QStringList excludeWords, QObject* parent = nullptr);

    /// \brief Returns a new string with excluded words removed.
    /// \param name name to remove excluded words from
    QString excludeWords(QString name);

    /// \brief Removes exclude dwords, changes "." and "_" to " " and removes all " - " at the end of the name.
    QString formatName(QString name, bool replaceDots = true, bool replaceUnderscores = true);

    /// \brief Removes the last part of a name, looking like " - cd1" or "_a"
    QString removeParts(QString name);

private:
    static bool lengthLessThan(const QString& s1, const QString& s2);

private:
    QStringList m_excludedWords;
};
