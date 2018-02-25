#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

#include "globals/Filter.h"

/**
 * @brief The MyLineEdit class
 * This widget can display a magnifier icon ("search") along with the text
 * and a loading spinner or reset icon
 */
class MyLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(LineEditType type READ type WRITE setType)
    Q_ENUMS(LineEditType)
public:
    enum LineEditType {
        TypeLoading,
        TypeClear
    };

    explicit MyLineEdit(QWidget *parent = nullptr);
    void setLoading(bool loading);
    void setType(LineEditType type);
    void addAdditionalStyleSheet(QString style);
    void setShowMagnifier(bool show);
    LineEditType type();
    void addFilter(Filter *filter);
    void clearFilters();
    void removeLastFilter();
    int paddingLeft();

signals:
    void keyUp();
    void keyDown();
    void focusOut();
    void focusIn();
    void backspaceInFront();

protected:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void focusInEvent(QFocusEvent *event);

private slots:
    void myTextChanged(QString text);
    void myClear();

private:
    QLabel *m_loadingLabel;
    QToolButton *m_clearButton;
    LineEditType m_type;
    bool m_showMagnifier;
    QLabel *m_magnifierLabel;
    QList<QLabel*> m_filterLabels;
    QStringList m_styleSheets;
    QLabel *m_moreLabel;
    int m_paddingLeft;
    void drawFilters();
};

#endif // MYLINEEDIT_H
