#pragma once

#include <QObject>
#include <QStringList>

/**
 * @brief The NameFormatter class
 */
class NameFormatter : public QObject
{
    Q_OBJECT
public:
    explicit NameFormatter(QObject* parent = nullptr);
    static NameFormatter* instance(QObject* parent = nullptr);

    QString excludeWords(QString name);
    QString formatName(QString name, bool replaceDots = true, bool replaceUnderscores = true);
    QString formatParts(QString name);

private slots:
    void onUpdateExcludeWords();

private:
    QStringList m_exWords;
    static bool lengthLessThan(const QString& s1, const QString& s2);
};
