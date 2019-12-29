#pragma once

#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "scrapers/tv_show/TheTvDb/ApiRequest.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

namespace thetvdb {

class Search : public QObject
{
    Q_OBJECT

public:
    explicit Search(QString language, QObject* parent = nullptr);
    void search(QString searchStr);

signals:
    void sigSearchDone(QVector<ScraperSearchResult>);

private:
    QNetworkAccessManager m_qnam;
    ApiRequest m_apiRequest;

    QVector<ScraperSearchResult> parseSearch(const QString& json);
    ScraperSearchResult parseSingleSearchResult(const QJsonObject& showObject);
    QUrl getShowSearchUrl(QString searchStr) const;
};

} // namespace thetvdb
