#pragma once

#include <QNetworkAccessManager>
#include <QObject>

class Update : public QObject
{
    Q_OBJECT
public:
    explicit Update(QObject* parent = nullptr);
    static Update* instance(QObject* parent = nullptr);

public slots:
    void checkForUpdate();

private slots:
    void onCheckFinished();

private:
    QNetworkAccessManager m_qnam;
    bool checkIfNewVersion(QString msg, QString& version);
};
