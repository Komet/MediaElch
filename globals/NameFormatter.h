#ifndef NAMEFORMATTER_H
#define NAMEFORMATTER_H

#include <QObject>
#include <QStringList>

/**
 * @brief The NameFormatter class
 */
class NameFormatter : public QObject
{
    Q_OBJECT
public:
    explicit NameFormatter(QObject *parent = 0);
    static NameFormatter *instance(QObject *parent = 0);

    QString excludeWords(QString name);
    QString formatName(QString name, bool replaceDots = true);
    QString formatParts(QString name);

private slots:
    void onUpdateExcludeWords();

private:
    QStringList m_exWords;
    static NameFormatter *m_instance;
};

#endif // NAMEFORMATTER_H
