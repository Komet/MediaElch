#ifndef UPDATE_H
#define UPDATE_H

#include <QNetworkAccessManager>
#include <QObject>

class Update : public QObject
{
    Q_OBJECT
public:
    explicit Update(QObject *parent = 0);
    static Update *instance(QObject *parent = 0);

public slots:
    void checkForUpdate();

signals:
    void sigNewVersion(QString);

private slots:
    void onCheckFinished();

private:
    QNetworkAccessManager m_qnam;
    bool checkIfNewVersion(QString msg, QString &version);
};

#endif // UPDATE_H
