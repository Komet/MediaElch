#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QTableView>

class MyTableView : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(int lastColumnWidth READ lastColumnWidth WRITE setLastColumnWidth)
    Q_PROPERTY(int firstColumnWidth READ firstColumnWidth WRITE setFirstColumnWidth)
public:
    explicit MyTableView(QWidget *parent = 0);
    int lastColumnWidth() const;
    void setLastColumnWidth(int &width);
    int firstColumnWidth() const;
    void setFirstColumnWidth(int &width);
    void setCaptureKeyPress(const bool &capture);

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void sigLeftEdge(bool);
    void sigKeyPress(QKeyEvent*);

protected:
    void mouseMoveEvent(QMouseEvent *event);

private:
    bool m_mouseInLeftEdge;
    bool m_captureKeyPress;
};

#endif // MYTABLEVIEW_H
