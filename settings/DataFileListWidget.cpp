#include "DataFileListWidget.h"

DataFileListWidget::DataFileListWidget(QWidget *parent) :
    QListWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *actionNew = new QAction(tr("Add"), this);
    QAction *actionDelete = new QAction(tr("Remove"), this);
    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(actionNew);
    m_contextMenu->addAction(actionDelete);

    setDragDropMode(QAbstractItemView::InternalMove);

    connect(actionNew, SIGNAL(triggered()), this, SLOT(addEntry()));
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(deleteEntry()));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

void DataFileListWidget::showContextMenu(QPoint point)
{
    m_contextMenu->exec(mapToGlobal(point));
}

void DataFileListWidget::addEntry()
{
    QListWidgetItem *item = new QListWidgetItem(tr("unnamed"));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    addItem(item);
}

void DataFileListWidget::deleteEntry()
{
    int row = currentRow();
    if (row < 0 || row >= count())
        return;
    delete takeItem(row);
}

void DataFileListWidget::setDataFiles(QList<DataFile> dataFiles, int type)
{
    m_type = type;
    clear();
    qSort(dataFiles.begin(), dataFiles.end(), DataFile::lessThan);
    foreach (DataFile dataFile, dataFiles) {
        QListWidgetItem *item = new QListWidgetItem(dataFile.fileName());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        addItem(item);
    }
}

QList<DataFile> DataFileListWidget::dataFiles()
{
    QList<DataFile> files;
    for (int i=0, n=count() ; i<n ; ++i) {
        QString fileName = item(i)->text();
        if (fileName.isEmpty() || fileName == "unnamed")
            continue;
        DataFile f(m_type, fileName, i);
        files.append(f);
    }

    return files;
}
