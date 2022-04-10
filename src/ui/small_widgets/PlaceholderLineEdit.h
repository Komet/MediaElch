#pragma once

#include <QComboBox>

/// \brief Combobox with comfort functions for custom placeholder strings.
/// \details This widget adds a custom QMenu to the standard context menu that
///          makes it easy to insert placeholders wrapped in "<" and ">".
///          Custom placeholders can be set through the property "placeholders".
class PlaceholderLineEdit : public QComboBox
{
    Q_OBJECT

    Q_PROPERTY(QStringList placeholders READ placeholders WRITE setPlaceholders)

public:
    PlaceholderLineEdit(QWidget* parent = nullptr);
    ~PlaceholderLineEdit() override = default;

    void setPlaceholders(const QStringList& placeholders);
    QStringList placeholders() const;

    /// \brief Add the given text if not already present and set it as the current one.
    void setText(const QString& text);
    /// \brief Alias for \see currentText
    QString text() const;

    /// \brief Clears all current items and adds the given ones.
    void setItems(const QStringList& items);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QString addDelimiters(const QString& placeholder);

private:
    QStringList m_placeholders;
};
