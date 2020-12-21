#pragma once

#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "network/NetworkManager.h"

#include <QObject>
#include <QQueue>

namespace mediaelch {
namespace scraper {

class TvTunes : public QObject
{
    Q_OBJECT
public:
    explicit TvTunes(QObject* parent = nullptr);
    void search(QString searchStr);

signals:
    void sigSearchDone(QVector<ScraperSearchResult>);

private slots:
    void onSearchFinished();
    void onDownloadUrlFinished();

private:
    mediaelch::network::NetworkManager m_network;
    QVector<ScraperSearchResult> m_results;
    QQueue<ScraperSearchResult> m_queue;
    QString m_searchStr;

private:
    /// \brief Parse the given HTML for TV tunes. Limits results to 50.
    QVector<ScraperSearchResult> parseSearch(QString html);
    void getNextDownloadUrl(QString searchStr = "");
};

} // namespace scraper
} // namespace mediaelch
