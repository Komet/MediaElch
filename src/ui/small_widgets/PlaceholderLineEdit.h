#pragma once

#include <QLineEdit>

/// \brief Line edit with confort functions for custom placeholder strings.
/// \details This widget adds a custom QMenu to the standard context menu that
///          makes it easy to insert placeholders wrapped in "<" and ">".
///          Custom placeholders can be set through the property "placeholders".
class PlaceholderLineEdit : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY(QStringList placeholders READ placeholders WRITE setPlaceholders)

public:
    using QLineEdit::QLineEdit;
    ~PlaceholderLineEdit() override = default;

    void setPlaceholders(const QStringList& placeholders);
    QStringList placeholders() const;

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QString addDelimiters(const QString& placeholder);

private:
    QStringList m_placeholders;
};
