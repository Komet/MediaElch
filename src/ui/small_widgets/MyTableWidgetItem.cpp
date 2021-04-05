#include "MyTableWidgetItem.h"

#include "log/Log.h"

#include "globals/Helper.h"
#include "settings/Settings.h"

MyTableWidgetItem::MyTableWidgetItem(QString text) : QTableWidgetItem(text), m_isSize{false}
{
}

MyTableWidgetItem::MyTableWidgetItem(QString text, double number) : QTableWidgetItem(text), m_isSize{false}
{
    setData(m_dataRole, number);
    setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

MyTableWidgetItem::MyTableWidgetItem(double number, bool isSize) :
    QTableWidgetItem(QString::number(number)), m_isSize{isSize}
{
    setData(m_dataRole, number);
    setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

QVariant MyTableWidgetItem::data(int role) const
{
    QLocale locale = Settings::instance()->advanced()->locale();
    const double fileSizeByte = QTableWidgetItem::data(Qt::DisplayRole).toDouble();

    if (role == Qt::DisplayRole && m_isSize) {
        const QString decimalSize = helper::formatFileSize(fileSizeByte, locale);
        return decimalSize;
    }

    if (role == Qt::ToolTipRole && m_isSize) {
        const QString binarySize = helper::formatFileSizeBinary(fileSizeByte, locale);
        return binarySize;
    }

    return QTableWidgetItem::data(role);
}

bool MyTableWidgetItem::operator<(const QTableWidgetItem& other) const
{
    if (!data(m_dataRole).toString().isEmpty()) {
        return data(m_dataRole).toDouble() < other.data(m_dataRole).toDouble();
    }

    return data(Qt::DisplayRole).toString() < other.data(Qt::DisplayRole).toString();
}
