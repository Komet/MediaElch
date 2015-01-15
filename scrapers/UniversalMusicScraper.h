#ifndef UNIVERSALMUSICSCRAPER_H
#define UNIVERSALMUSICSCRAPER_H

#include <QComboBox>
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
    void loadData(QString mbId, Album *album, QList<int> infos);
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
    void onTadbAlbumLoadFinished();
    void onTadbArtistLoadFinished();
    void onAmAlbumLoadFinished();
    void onAmArtistLoadFinished();
    void onAmBiographyLoadFinished();
    void onArtistRelsFinished();
    void onAlbumRelsFinished();

private:
    QString m_tadbApiKey;
    QNetworkAccessManager m_qnam;
    QString m_language;
    QString m_prefer;
    QString m_lastScraper;
    QWidget *m_widget;
    QComboBox *m_box;
    QComboBox *m_preferBox;

    QNetworkAccessManager *qnam();
    void loadTadbData(QString mbId, Artist *artist, QList<int> infos);
    void loadTadbData(QString mbId, Album *album, QList<int> infos);
    void loadAmData(QString allMusicId, Artist *artist, QList<int> infos);
    void loadAmData(QString allMusicId, Album *album, QList<int> infos);
    void loadAmBiography(QString allMusicId, Artist *artist, QList<int> infos);
    void parseAndAssignTadbInfos(QString json, Artist *artist, QList<int> infos);
    void parseAndAssignTadbInfos(QString json, Album *album, QList<int> infos);
    void parseAndAssignAmInfos(QString html, Artist *artist, QList<int> infos);
    void parseAndAssignAmInfos(QString html, Album *album, QList<int> infos);
    void parseAndAssignAmBiography(QString html, Artist *artist, QList<int> infos);
    QString trim(QString text);
    bool shouldLoad(int info, QList<int> infos, Artist *artist);
    bool shouldLoad(int info, QList<int> infos, Album *album);
    bool infosLeft(QList<int> infos, Artist *artist);
    bool infosLeft(QList<int> infos, Album *album);
};

#endif // UNIVERSALMUSICSCRAPER_H
