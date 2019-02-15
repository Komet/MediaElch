#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QUrl>

namespace MediaElch {

class JsonPostRequest : public QObject
{
    Q_OBJECT

public:
    explicit JsonPostRequest(QUrl url, QJsonObject body);

signals:
    void jsonParsed(QJsonDocument& document);
};

} // namespace MediaElch
