#ifndef NAMEFORMATTER_H
#define NAMEFORMATTER_H

#include <QObject>
#include <QStringList>

class NameFormatter : public QObject
{
    Q_OBJECT
public:
    explicit NameFormatter(QObject *parent = 0);
    static NameFormatter *instance(QObject *parent = 0);

    QString excludeWords(QString name);
    QString formatName(QString name);
    QString formatParts(QString name);


private:
    QStringList m_exWords;
    static NameFormatter *m_instance;
    
signals:
    
public slots:
    
};

#endif // NAMEFORMATTER_H
