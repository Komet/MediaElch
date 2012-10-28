#ifndef DATAFILELISTWIDGET_H
#define DATAFILELISTWIDGET_H

#include <QListWidget>
#include <QMenu>
#include "settings/DataFile.h"

class DataFileListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit DataFileListWidget(QWidget *parent = 0);
    void setDataFiles(QList<DataFile> dataFiles, int type);
    QList<DataFile> dataFiles();

private slots:
    void showContextMenu(QPoint point);
    void addEntry();
    void deleteEntry();

private:
    int m_type;
    QMenu *m_contextMenu;
};

#endif // DATAFILELISTWIDGET_H
