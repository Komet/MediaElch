#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

struct Actor
{
    QString name;
    QString role;
    QString thumb;
    QByteArray image;
    QString id;
    int order = 0; // used by Kodi NFO
    bool imageHasChanged = false;
};

Q_DECLARE_METATYPE(Actor*)
Q_DECLARE_METATYPE(QString*)
Q_DECLARE_METATYPE(QVector<int>)
