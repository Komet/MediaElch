#ifndef ALPHABETICALLIST_H
#define ALPHABETICALLIST_H

#include <QPaintEvent>
#include <QPointer>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

#include "globals/Globals.h"
#include "smallWidgets/MyTableView.h"

class AlphabeticalList : public QWidget
{
    Q_OBJECT
public:
    explicit AlphabeticalList(QWidget *parent = 0, MyTableView *parentTableView = nullptr);
    void setTopSpace(const int space);
    void setBottomSpace(const int space);
    void setRightSpace(const int space);
    void setLeftSpace(const int space);
    void setAlphas(QStringList alphas);
    int leftSpace() const;

public slots:
    void show();
    void hide();
    void adjustSize();

signals:
    void sigAlphaClicked(QString);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void onAlphaClicked();

private:
    QPointer<QPropertyAnimation> m_inAnim;
    QPointer<QPropertyAnimation> m_outAnim;
    QVBoxLayout *m_layout;
    int m_bottomSpace;
    int m_topSpace;
    int m_rightSpace;
    int m_leftSpace;
    int m_animDuration;
    MyTableView *m_tableView;
};

#endif // ALPHABETICALLIST_H
