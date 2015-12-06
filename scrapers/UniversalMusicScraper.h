#ifndef UNIVERSALMUSICSCRAPER_H
#define UNIVERSALMUSICSCRAPER_H

#include <QComboBox>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>
#include "../data/MusicScraperInterface.h"

class UniversalMusicScraper : public MusicScraperInterface
{
    Q_OBJECT
public:
    explicit UniversalMusicScraper(QObject *parent = 0);

    QString name();
    QString identifier();
    void searchAlbum(QString artistName, QString searchStr);
    void searchArtist(QString searchStr);
    void loadData(QString mbId, Artist *artist, QList<int> infos);
    void loadData(QString mbAlbumId, QString mbReleaseGroupId, Album *album, QList<int> infos);
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QList<int> scraperSupports();
    QWidget *settingsWidget();

signals:
    void sigSearchDone(QList<ScraperSearchResult>);

private slots:
    void onSearchArtistFinished();
    void onSearchAlbumFinished();
    void onArtistRelsFinished();
    void onAlbumRelsFinished();
    void onArtistLoadFinished();
    void onAlbumLoadFinished();

private:
    struct DownloadElement {
        QString source;
        QString type;
        QUrl url;
        bool downloaded;
        QString contents;
    };

    QString m_tadbApiKey;
    QNetworkAccessManager m_qnam;
    QString m_language;
    QString m_prefer;
    QWidget *m_widget;
    QComboBox *m_box;
    QComboBox *m_preferBox;
    QMap<Artist*, QList<DownloadElement> > m_artistDownloads;
    QMap<Album*, QList<DownloadElement> > m_albumDownloads;
    QMutex m_artistMutex;
    QMutex m_albumMutex;

    QNetworkAccessManager *qnam();
    QString trim(QString text);
    bool shouldLoad(int info, QList<int> infos, Artist *artist);
    bool shouldLoad(int info, QList<int> infos, Album *album);
    bool infosLeft(QList<int> infos, Artist *artist);
    bool infosLeft(QList<int> infos, Album *album);
    void appendDownloadElement(Artist *artist, QString source, QString type, QUrl url);
    void appendDownloadElement(Album *album, QString source, QString type, QUrl url);
    void parseAndAssignMusicbrainzInfos(QString xml, Album *album, QList<int> infos);
    void parseAndAssignTadbInfos(QString json, Artist *artist, QList<int> infos);
    void parseAndAssignTadbInfos(QString json, Album *album, QList<int> infos);
    void parseAndAssignTadbDiscography(QString json, Artist *artist, QList<int> infos);
    void parseAndAssignAmInfos(QString html, Artist *artist, QList<int> infos);
    void parseAndAssignAmInfos(QString html, Album *album, QList<int> infos);
    void parseAndAssignAmBiography(QString html, Artist *artist, QList<int> infos);
    void parseAndAssignAmDiscography(QString html, Artist *artist, QList<int> infos);
    void parseAndAssignDiscogsInfos(QString html, Artist *artist, QList<int> infos);
    void parseAndAssignDiscogsInfos(QString html, Album *album, QList<int> infos);
    void processDownloadElement(DownloadElement elem, Artist *artist, QList<int> infos);
    void processDownloadElement(DownloadElement elem, Album *album, QList<int> infos);
};

#endif // UNIVERSALMUSICSCRAPER_H
