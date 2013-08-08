#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QTableView>
#include <QTimer>

#include "smallWidgets/SearchOverlay.h"

class MyTableView : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(int lastColumnWidth READ lastColumnWidth WRITE setLastColumnWidth)
    Q_PROPERTY(int firstColumnWidth READ firstColumnWidth WRITE setFirstColumnWidth)
    Q_PROPERTY(bool useSearchOverlay READ useSearchOverlay WRITE setUseSearchOverlay DESIGNABLE true)

public:
    explicit MyTableView(QWidget *parent = 0);
    int lastColumnWidth() const;
    void setLastColumnWidth(int &width);
    int firstColumnWidth() const;
    void setFirstColumnWidth(int &width);
    void setUseSearchOverlay(const bool &useSearchOverlay);
    bool useSearchOverlay() const;

protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:
    void sigLeftEdge(bool);

private slots:
    void onSearchOverlayTimeout();

private:
    bool m_mouseInLeftEdge;
    bool m_useSearchOverlay;
    QString m_currentSearchText;
    QTimer m_searchOverlayTimer;
    SearchOverlay *m_searchOverlay;
};

#endif // MYTABLEVIEW_H
