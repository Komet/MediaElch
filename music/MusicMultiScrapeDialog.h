#ifndef MUSICMULTISCRAPEDIALOG_H
#define MUSICMULTISCRAPEDIALOG_H

#include <QDialog>

#include "Album.h"
#include "Artist.h"

namespace Ui {
class MusicMultiScrapeDialog;
}

class MusicMultiScrapeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MusicMultiScrapeDialog(QWidget *parent = 0);
    ~MusicMultiScrapeDialog();
    static MusicMultiScrapeDialog *instance(QWidget *parent = 0);
    void setItems(QList<Artist*> artists, QList<Album*> albums);

public slots:
    int exec();
    void reject();
    void accept();

private slots:
    void onChkToggled();
    void onChkAllToggled(bool toggled);
    void onStartScraping();
    void onScrapingFinished();
    void onSearchFinished(QList<ScraperSearchResult> results);
    void scrapeNext();
    void onProgress(Artist *artist, int current, int maximum);
    void onProgress(Album *album, int current, int maximum);

private:
    Ui::MusicMultiScrapeDialog *ui;

    struct QueueItem {
        Artist *artist;
        Album *album;
    };

    void disconnectScrapers();
    bool isExecuted();

    QQueue<QueueItem> m_queue;
    bool m_executed;
    Artist *m_currentArtist;
    Album *m_currentAlbum;
    QList<int> m_artistInfosToLoad;
    QList<int> m_albumInfosToLoad;
    QList<Artist*> m_artists;
    QList<Album*> m_albums;
    MusicScraperInterface *m_scraperInterface;
};

#endif // MUSICMULTISCRAPEDIALOG_H
