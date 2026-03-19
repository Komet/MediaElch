#pragma once

#include <QObject>

#include "settings/UpdateCheck.h"

class Update : public QObject
{
    Q_OBJECT

public:
    explicit Update(QObject* parent = nullptr);
    static Update* instance(QObject* parent = nullptr);

public slots:
    void checkForUpdate();

private slots:
    void onUpdateCheckFinished(mediaelch::UpdateCheck::Result result);

private:
    mediaelch::UpdateCheck m_updateCheck;
};
