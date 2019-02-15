#pragma once

#include "globals/Globals.h"
#include "ui/small_widgets/MyTableView.h"

#include <QPaintEvent>
#include <QPointer>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

class AlphabeticalList : public QWidget
{
    Q_OBJECT
public:
    explicit AlphabeticalList(QWidget* parent = nullptr, MyTableView* parentTableView = nullptr);
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
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onAlphaClicked();

private:
    void stopAnimation();

    QPointer<QPropertyAnimation> m_inAnim;
    QPointer<QPropertyAnimation> m_outAnim;
    QVBoxLayout* m_layout;
    int m_bottomSpace{10};
    int m_topSpace{10};
    int m_rightSpace{10};
    int m_leftSpace{10};
    int m_animDuration{100};
    MyTableView* m_tableView;
};
