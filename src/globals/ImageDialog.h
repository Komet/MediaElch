#pragma once

#include "globals/Globals.h"
#include "tvShows/EpisodeNumber.h"
#include "tvShows/SeasonNumber.h"

#include <QDialog>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QUrl>
#include <QWidget>

namespace Ui {
class ImageDialog;
}

class Album;
class Artist;
class Concert;
class ImageLabel;
class Movie;
class TvShow;
class TvShowEpisode;
class ImageProviderInterface;

/**
 * @brief The ImageDialog class
 * Displays a set of images
 */
class ImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageDialog(QWidget* parent = nullptr);
    ~ImageDialog() override;
    static ImageDialog* instance(QWidget* parent = nullptr);
    void setDownloads(QVector<Poster> downloads, bool initial = true);
    QUrl imageUrl();
    QVector<QUrl> imageUrls();
    void setImageType(ImageType type);
    void setItemType(ItemType type);
    void setMultiSelection(const bool& enable);
    void setMovie(Movie* movie);
    void setConcert(Concert* concert);
    void setTvShow(TvShow* show);
    void setSeason(SeasonNumber season);
    void setTvShowEpisode(TvShowEpisode* episode);
    void setArtist(Artist* artist);
    void setAlbum(Album* album);
    void clear();
    void cancelDownloads();

public slots:
    void accept() override;
    void reject() override;
    int exec() override;
    int exec(ImageType type);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void downloadFinished();
    void startNextDownload();
    void imageClicked(int row, int col);
    void chooseLocalImage();
    void onImageDropped(QUrl url);
    void onPreviewSizeChange(int value);
    void onZoomIn();
    void onZoomOut();
    void onSearch(bool onlyFirstResult = false);
    void onProviderChanged(int index);
    void onSearchFinished(QVector<ScraperSearchResult> results);
    void onResultClicked(QTableWidgetItem* item);
    void onProviderImagesLoaded(QVector<Poster> images);
    void onImageClosed(const QString& url);
    void updateSourceLink();

private:
    Ui::ImageDialog* ui;

    /**
     * @brief The DownloadElement struct
     */
    struct DownloadElement
    {
        QUrl thumbUrl;
        QUrl originalUrl;
        QPixmap pixmap;
        QPixmap scaledPixmap;
        bool downloaded;
        ImageLabel* cellWidget;
        QSize resolution;
        QString hint;
    };

    QNetworkAccessManager m_qnam;
    int m_currentDownloadIndex;
    QNetworkReply* m_currentDownloadReply;
    ImageType m_imageType;
    QVector<DownloadElement> m_elements;
    QUrl m_imageUrl;
    QVector<QUrl> m_imageUrls;
    ImageType m_type;
    QVector<ImageProviderInterface*> m_providers;
    Concert* m_concert;
    Movie* m_movie;
    TvShow* m_tvShow;
    TvShowEpisode* m_tvShowEpisode;
    ItemType m_itemType;
    QVector<Poster> m_defaultElements;
    ImageProviderInterface* m_currentProvider;
    SeasonNumber m_season;
    EpisodeNumber m_episode;
    bool m_multiSelection;
    Artist* m_artist;
    Album* m_album;

    QNetworkAccessManager* qnam();
    void renderTable();
    int calcColumnCount();
    int getColumnWidth();
    void loadImagesFromProvider(QString id);
    void clearSearch();
    QString formatSearchText(const QString& text);
};
