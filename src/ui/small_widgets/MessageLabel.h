#pragma once

#include <QLabel>

class MessageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MessageLabel(QWidget* parent = nullptr, unsigned int alignment = Qt::AlignLeft | Qt::AlignVCenter);
    void setErrorMessage(const QString& text);
    void setSuccessMessage(const QString& text);
    void setStatusMessage(const QString& text);

public slots:
    void clear();
};
