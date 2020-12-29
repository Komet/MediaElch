#include "ComboDelegate.h"

#include <QComboBox>
#include <QDebug>
#include <QLineEdit>

#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"

ComboDelegate::ComboDelegate(QObject* parent, MainWidgets widget, ComboDelegateType type) :
    QItemDelegate(parent), m_widget{widget}, m_type{type}
{
}

/**
 * \brief Sets up a combo box with available genres
 */
QWidget*
ComboDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto* box = new QComboBox(parent);
    box->setEditable(true);
    return box;
}

void ComboDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    auto* box = dynamic_cast<QComboBox*>(editor);
    QStringList items;
    const auto& movies = Manager::instance()->movieModel()->movies();
    const auto& shows = Manager::instance()->tvShowModel()->tvShows();

    if (m_widget == MainWidgets::Movies && m_type == ComboDelegateType::Genres) {
        for (Movie* movie : movies) {
            for (const QString& genre : movie->genres()) {
                if (!genre.isEmpty() && !items.contains(genre)) {
                    items.append(genre);
                }
            }
        }
    } else if (m_widget == MainWidgets::Movies && m_type == ComboDelegateType::Countries) {
        for (Movie* movie : movies) {
            for (const QString& country : movie->countries()) {
                if (!country.isEmpty() && !items.contains(country)) {
                    items.append(country);
                }
            }
        }
    } else if (m_widget == MainWidgets::Movies && m_type == ComboDelegateType::Studios) {
        for (Movie* movie : movies) {
            for (const QString& studio : movie->studios()) {
                if (!studio.isEmpty() && !items.contains(studio)) {
                    items.append(studio);
                }
            }
        }
    } else if (m_widget == MainWidgets::Concerts && m_type == ComboDelegateType::Genres) {
        const auto& concerts = Manager::instance()->concertModel()->concerts();
        for (Concert* concert : concerts) {
            for (const QString& genre : concert->genres()) {
                if (!genre.isEmpty() && !items.contains(genre)) {
                    items.append(genre);
                }
            }
        }
    } else if (m_widget == MainWidgets::TvShows && m_type == ComboDelegateType::Genres) {
        for (TvShow* show : shows) {
            for (const QString& genre : show->genres()) {
                if (!genre.isEmpty() && !items.contains(genre)) {
                    items.append(genre);
                }
            }
        }
    } else if (m_widget == MainWidgets::TvShows && m_type == ComboDelegateType::Directors) {
        for (TvShow* show : shows) {
            for (TvShowEpisode* episode : show->episodes()) {
                for (const QString& director : episode->directors()) {
                    if (!director.isEmpty() && !items.contains(director)) {
                        items.append(director);
                    }
                }
            }
        }
    } else if (m_widget == MainWidgets::TvShows && m_type == ComboDelegateType::Writers) {
        for (TvShow* show : shows) {
            for (TvShowEpisode* episode : show->episodes()) {
                for (const QString& writer : episode->writers()) {
                    if (!writer.isEmpty() && !items.contains(writer)) {
                        items.append(writer);
                    }
                }
            }
        }
    }
    std::sort(items.begin(), items.end(), LocaleStringCompare());
    box->addItems(items);
    box->lineEdit()->setText(value);
}

void ComboDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* box = dynamic_cast<QComboBox*>(editor);
    QString value = box->currentText();
    model->setData(index, value, Qt::EditRole);
}

void ComboDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QSize ComboDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int width = QItemDelegate::sizeHint(option, index).width();
    return {width, 25};
}
