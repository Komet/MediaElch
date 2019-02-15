#pragma once

#include <QTableWidgetItem>

class MyTableWidgetItem : public QTableWidgetItem
{
public:
    explicit MyTableWidgetItem(QString text);
    MyTableWidgetItem(QString text, qreal number);
    MyTableWidgetItem(int64_t number, bool isSize = false);
    MyTableWidgetItem(int number, bool isSize = false);
    QVariant data(int role) const;
    virtual bool operator<(const QTableWidgetItem& other) const;

private:
    bool m_isSize;
};
