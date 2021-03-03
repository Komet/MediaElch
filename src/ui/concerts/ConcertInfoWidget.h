#pragma once

#include "concerts/ConcertController.h"

#include <QDate>
#include <QDateTime>
#include <QPointer>
#include <QString>
#include <QWidget>
#include <chrono>
#include <memory>

namespace Ui {
class ConcertInfoWidget;
}

class ClosableImage;

/**
 * \brief The ConcertInfoWidget class
 */
class ConcertInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertInfoWidget(QWidget* parent = nullptr);
    ~ConcertInfoWidget() override;

    void setConcertController(ConcertController* controller);
    void updateConcertInfo();

    void setRuntime(std::chrono::minutes runtime);

signals:
    void concertNameChanged(QString concertName);
    void infoChanged();

private slots:
    void onConcertTitleChanged(QString concertName);

    void onTitleChange(QString text);
    void onOriginalTitleChange(QString text);
    void onTmdbIdChanged(QString text);
    void onImdbIdChanged(QString text);
    void onArtistChange(QString text);
    void onAlbumChange(QString text);
    void onTaglineChange(QString text);
    void onRatingChange(double value);
    void onUserRatingChange(double value);
    void onReleasedChange(QDate date);
    void onRuntimeChange(int value);
    void onCertificationChange(QString text);
    void onTrailerChange(QString text);
    void onWatchedClicked();
    void onPlayCountChange(int value);
    void onLastWatchedChange(QDateTime dateTime);
    void onOverviewChange();

private:
    void clear();

    std::unique_ptr<Ui::ConcertInfoWidget> ui;
    QPointer<ConcertController> m_concertController = nullptr;
};
