#pragma once

#include "network/NetworkManager.h"

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
    mediaelch::network::NetworkManager m_network;
    bool checkIfNewVersion(QString xmlString, QString& version, QString& downloadUrl);
};
