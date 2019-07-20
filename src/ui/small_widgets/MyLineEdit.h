#pragma once

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
    enum LineEditType
    {
        TypeLoading,
        TypeClear
    };

    explicit MyLineEdit(QWidget* parent = nullptr);
    void setLoading(bool loading);
    void setType(LineEditType type);
    void addAdditionalStyleSheet(QString style);
    void setShowMagnifier(bool show);
    LineEditType type();
    void addFilter(Filter* filter);
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
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;

private slots:
    void myTextChanged(QString text);
    void myClear();

private:
    QLabel* m_loadingLabel = nullptr;
    QToolButton* m_clearButton = nullptr;
    LineEditType m_type = LineEditType::TypeLoading;
    bool m_showMagnifier = false;
    QLabel* m_magnifierLabel = nullptr;
    QVector<QLabel*> m_filterLabels;
    QStringList m_styleSheets;
    QLabel* m_moreLabel = nullptr;
    int m_paddingLeft = 0;
    void drawFilters();
};
