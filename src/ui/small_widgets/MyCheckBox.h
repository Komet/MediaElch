#pragma once

#include <QCheckBox>
#include <QVariant>

class MyCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit MyCheckBox(QWidget* parent = nullptr);
    void setMyData(const QVariant& myData);
    QVariant myData() const;

private:
    QVariant m_myData;
};
