#ifndef THEAUDIODB_H
#define THEAUDIODB_H

#include <QComboBox>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>
#include "../data/MusicScraperInterface.h"

class TheAudioDb : public MusicScraperInterface
{
    Q_OBJECT
public:
    explicit TheAudioDb(QObject *parent = 0);

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
    void onAlbumLoadFinished();
    void onArtistLoadFinished();

private:
    QString m_apiKey;
    QNetworkAccessManager m_qnam;
    QString m_language;
    QWidget *m_widget;
    QComboBox *m_box;

    QNetworkAccessManager *qnam();
    void parseAndAssignInfos(QString json, Artist *artist, QList<int> infos);
    void parseAndAssignInfos(QString json, Album *album, QList<int> infos);
};

#endif // THEAUDIODB_H
