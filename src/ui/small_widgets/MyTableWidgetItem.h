#pragma once

#include <QTableWidgetItem>

class MyTableWidgetItem : public QTableWidgetItem
{
public:
    explicit MyTableWidgetItem(QString text);
    MyTableWidgetItem(QString text, double number);
    MyTableWidgetItem(double number, bool isSize = false);

    QVariant data(int role) const;
    virtual bool operator<(const QTableWidgetItem& other) const;

private:
    bool m_isSize;

    const int m_dataRole = 1000;
};
