#pragma once

#include "globals/Globals.h"
#include "globals/Poster.h"
#include "globals/ScraperResult.h"
#include "scrapers/image/ImageProviderInterface.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"

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

/// Displays a set of images
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
    void onSearchWithAllResults();
    void onProviderChanged(int index);
    void onSearchFinished(QVector<ScraperSearchResult> results, ScraperSearchError error);
    void onResultClicked(QTableWidgetItem* item);
    void onProviderImagesLoaded(QVector<Poster> images, ScraperLoadError error);
    void onImageClosed(const QString& url);
    void updateSourceLink();

private:
    Ui::ImageDialog* ui;

    struct DownloadElement
    {
        QUrl thumbUrl;
        QUrl originalUrl;
        QPixmap pixmap;
        QPixmap scaledPixmap;
        bool downloaded = false;
        ImageLabel* cellWidget = nullptr;
        QSize resolution;
        QString hint;
    };

    struct DataRole
    {
        constexpr static int providerPointer = Qt::UserRole;
        constexpr static int isDefaultProvider = Qt::UserRole + 1;
    };

    QNetworkAccessManager m_qnam;
    int m_currentDownloadIndex = 0;
    QNetworkReply* m_currentDownloadReply = nullptr;
    ImageType m_imageType = ImageType::None;
    QVector<DownloadElement> m_elements;
    QUrl m_imageUrl;
    QVector<QUrl> m_imageUrls;
    ImageType m_type = ImageType::None;
    QVector<ImageProviderInterface*> m_providers;
    Concert* m_concert = nullptr;
    Movie* m_movie = nullptr;
    TvShow* m_tvShow = nullptr;
    TvShowEpisode* m_tvShowEpisode = nullptr;
    ItemType m_itemType = ItemType::Movie;
    QVector<Poster> m_defaultElements;
    ImageProviderInterface* m_currentProvider = nullptr;
    SeasonNumber m_season;
    EpisodeNumber m_episode;
    bool m_multiSelection = false;
    Artist* m_artist = nullptr;
    Album* m_album = nullptr;

    QNetworkAccessManager* qnam();
    void renderTable();
    int calcColumnCount();
    int getColumnWidth();
    void loadImagesFromProvider(QString id);
    void clearSearch();
    QString formatSearchText(const QString& text);

    void showError(const QString& message);
    void showSuccess(const QString& message);

    bool hasDefaultImages() const { return !m_defaultElements.isEmpty(); }
    bool hasImageProvider() const { return !m_providers.isEmpty(); }
};
