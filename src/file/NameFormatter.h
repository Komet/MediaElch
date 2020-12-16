#pragma once

#include <QObject>
#include <QStringList>

class NameFormatter : public QObject
{
    Q_OBJECT
public:
    explicit NameFormatter(QObject* parent = nullptr);

    QString excludeWords(QString name);
    QString formatName(QString name, bool replaceDots = true, bool replaceUnderscores = true);
    QString formatParts(QString name);

private:
    void updateExcludeWords();
    static bool lengthLessThan(const QString& s1, const QString& s2);

private:
    QStringList m_exWords;
};
