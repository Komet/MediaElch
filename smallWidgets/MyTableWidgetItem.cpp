#include "MyTableWidgetItem.h"

#include <QDebug>
#include "globals/Helper.h"

MyTableWidgetItem::MyTableWidgetItem(QString text) :
    QTableWidgetItem(text)
{
    m_isSize = false;
}

MyTableWidgetItem::MyTableWidgetItem(QString text, qreal number) :
    QTableWidgetItem(number)
{
    m_isSize = false;
    setData(1000, (qreal)number);
    setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setText(text);
}

MyTableWidgetItem::MyTableWidgetItem(qreal number, bool isSize) :
    QTableWidgetItem(number),
    m_isSize(isSize)
{
    setData(1000, (qreal)number);
    setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setText(QString::number(number));
}

QVariant MyTableWidgetItem::data(int role) const
{
    if (role == Qt::DisplayRole && m_isSize)
        return Helper::instance()->formatFileSize(QTableWidgetItem::data(Qt::DisplayRole).toReal());

    return QTableWidgetItem::data(role);
}

bool MyTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    if (!data(1000).toString().isEmpty())
        return data(1000).toReal() < other.data(1000).toReal();

    return data(Qt::DisplayRole).toString() < other.data(Qt::DisplayRole).toString();
}
