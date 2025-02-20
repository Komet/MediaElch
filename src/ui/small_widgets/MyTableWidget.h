#pragma once

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QTableWidget>
#include <QUrl>

/**
 * \brief The MyTableWidget class can handle drag and drop events
 */
class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;

signals:
    /// Emitted when an image was dropped.
    void sigDroppedImage(QUrl);
};
