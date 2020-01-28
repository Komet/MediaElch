#include "MyTableWidgetItem.h"

#include <QDebug>

#include "globals/Helper.h"

MyTableWidgetItem::MyTableWidgetItem(QString text) : QTableWidgetItem(text), m_isSize{false}
{
}

MyTableWidgetItem::MyTableWidgetItem(QString text, qreal number) :
    QTableWidgetItem(static_cast<int>(number)), m_isSize{false}
{
    setData(1000, number);
    setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setText(text);
}

MyTableWidgetItem::MyTableWidgetItem(int64_t number, bool isSize) : MyTableWidgetItem(static_cast<int>(number), isSize)
{
}

MyTableWidgetItem::MyTableWidgetItem(int number, bool isSize) : QTableWidgetItem(number), m_isSize{isSize}
{
    setData(1000, number);
    setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setText(QString::number(number));
}

QVariant MyTableWidgetItem::data(int role) const
{
    if (role == Qt::DisplayRole && m_isSize) {
        return helper::formatFileSize(QTableWidgetItem::data(Qt::DisplayRole).toLongLong());
    }

    return QTableWidgetItem::data(role);
}

bool MyTableWidgetItem::operator<(const QTableWidgetItem& other) const
{
    if (!data(1000).toString().isEmpty()) {
        return data(1000).toReal() < other.data(1000).toReal();
    }

    return data(Qt::DisplayRole).toString() < other.data(Qt::DisplayRole).toString();
}
