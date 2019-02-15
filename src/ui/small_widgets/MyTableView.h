#pragma once

#include <QKeyEvent>
#include <QMouseEvent>
#include <QTableView>
#include <QTimer>

#include "ui/small_widgets/SearchOverlay.h"

class MyTableView : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(int lastColumnWidth READ lastColumnWidth WRITE setLastColumnWidth)
    Q_PROPERTY(int firstColumnWidth READ firstColumnWidth WRITE setFirstColumnWidth)
    Q_PROPERTY(bool useSearchOverlay READ useSearchOverlay WRITE setUseSearchOverlay DESIGNABLE true)

public:
    explicit MyTableView(QWidget* parent = nullptr);
    int lastColumnWidth() const;
    void setLastColumnWidth(int& width);
    int firstColumnWidth() const;
    void setFirstColumnWidth(int& width);
    void setUseSearchOverlay(const bool& useSearchOverlay);
    bool useSearchOverlay() const;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    void sigLeftEdge(bool);

private slots:
    void onSearchOverlayTimeout();

private:
    bool m_mouseInLeftEdge = false;
    bool m_useSearchOverlay = false;
    QString m_currentSearchText;
    QTimer m_searchOverlayTimer;
    SearchOverlay* m_searchOverlay;
};
