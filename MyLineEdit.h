#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

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

    explicit MyLineEdit(QWidget *parent = 0);
    void setLoading(bool loading);
    void setType(LineEditType type);
    LineEditType type();

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void myTextChanged(QString text);
    void myClear();

private:
    QLabel *m_loadingLabel;
    QToolButton *m_clearButton;
    LineEditType m_type;
};

#endif // MYLINEEDIT_H
