#ifndef ALPHABETICALLIST_H
#define ALPHABETICALLIST_H

#include <QPaintEvent>
#include <QPointer>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>
#include "globals/Globals.h"

class AlphabeticalList : public QWidget
{
    Q_OBJECT
public:
    explicit AlphabeticalList(QWidget *parent = 0);
    void setTopSpace(const int space);
    void setBottomSpace(const int space);
    void setRightSpace(const int space);
    void setAlphas(QStringList alphas);

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
    QPointer<QPropertyAnimation> inAnim;
    QPointer<QPropertyAnimation> outAnim;
    QVBoxLayout *m_layout;
    int m_bottomSpace;
    int m_topSpace;
    int m_rightSpace;
    int m_animDuration;
};

#endif // ALPHABETICALLIST_H
