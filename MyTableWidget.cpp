#include "MyTableWidget.h"

#include <QDebug>
#include <QMimeData>

MyTableWidget::MyTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
}

void MyTableWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MyTableWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}


void MyTableWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if( mimeData->hasUrls() ) {
        if (mimeData->urls().size() > 0) {
            QUrl url = mimeData->urls().at(0);
            QStringList filters;
            filters << ".jpg" << ".JPG" << ".jpeg" << ".JPeg" << ".Jpg" << ".png" << ".PNG";
            foreach (const QString &filter, filters) {
                if (url.toString().endsWith(filter)) {
                    emit sigDroppedImage(url);
                    return;
                }
            }
        }
    }
}
