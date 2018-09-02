#ifndef MESSAGELABEL_H
#define MESSAGELABEL_H

#include <QLabel>

class MessageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MessageLabel(QWidget *parent = nullptr, unsigned int alignment = Qt::AlignLeft | Qt::AlignVCenter);
    void setErrorMessage(const QString &text);
    void setSuccessMessage(const QString &text);

public slots:
    void clear();
};

#endif // MESSAGELABEL_H
