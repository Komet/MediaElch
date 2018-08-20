#include "MyTableWidget.h"

#include <QDebug>
#include <QMimeData>

#include "globals/Globals.h"

/**
 * @brief MyTableWidget::MyTableWidget
 * @param parent
 */
MyTableWidget::MyTableWidget(QWidget *parent) : QTableWidget(parent)
{
}

/**
 * @brief MyTableWidget::dragMoveEvent
 * @param event
 */
void MyTableWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

/**
 * @brief MyTableWidget::dragEnterEvent
 * @param event
 */
void MyTableWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

/**
 * @brief If an image was dropped the sigDroppedImage signal is emitted
 * @param event
 */
void MyTableWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        if (!mimeData->urls().empty()) {
            QUrl url = mimeData->urls().at(0);
            QStringList filters{".jpg", ".JPG", ".jpeg", ".JPeg", ".Jpg", ".png", ".PNG"};
            for (const QString &filter : filters) {
                if (url.toString().endsWith(filter)) {
                    emit sigDroppedImage(url);
                    return;
                }
            }
        }
    }
}
