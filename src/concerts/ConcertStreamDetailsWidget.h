#ifndef CONCERT_STREAMDETAILS_WIDGET_H
#define CONCERT_STREAMDETAILS_WIDGET_H

#include "concerts/ConcertController.h"

#include <QLineEdit>
#include <QList>
#include <QPointer>
#include <QWidget>
#include <chrono>
#include <memory>

namespace Ui {
class ConcertStreamDetailsWidget;
}

class ConcertStreamDetailsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertStreamDetailsWidget(QWidget *parent = nullptr);
    ~ConcertStreamDetailsWidget() override;

    void setConcertController(ConcertController *controller);
    void updateConcertInfo();

signals:
    void streamDetailsChanged();
    void runtimeChanged(std::chrono::minutes);

private slots:
    void onReloadStreamDetails();

    void onStreamDetailsEdited();
    void updateStreamDetails(bool reloadFromFile = false);

private:
    void clear();

    std::unique_ptr<Ui::ConcertStreamDetailsWidget> ui;
    QPointer<ConcertController> m_concertController = nullptr;
    QList<QWidget *> m_streamDetailsWidgets;
    QList<QList<QLineEdit *>> m_streamDetailsAudio;
    QList<QList<QLineEdit *>> m_streamDetailsSubtitles;
};

#endif // CONCERT_STREAMDETAILS_WIDGET_H
