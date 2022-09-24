#include "ComboDelegate.h"

#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "utils/Meta.h"

#include <QComboBox>
#include <QLineEdit>

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
            const auto genres = movie->genres();
            for (const QString& genre : genres) {
                if (!genre.isEmpty() && !items.contains(genre)) {
                    items.append(genre);
                }
            }
        }
    } else if (m_widget == MainWidgets::Movies && m_type == ComboDelegateType::Countries) {
        for (Movie* movie : movies) {
            const auto countries = movie->countries();
            for (const QString& country : countries) {
                if (!country.isEmpty() && !items.contains(country)) {
                    items.append(country);
                }
            }
        }
    } else if (m_widget == MainWidgets::Movies && m_type == ComboDelegateType::Studios) {
        for (Movie* movie : movies) {
            const auto studios = movie->studios();
            for (const QString& studio : studios) {
                if (!studio.isEmpty() && !items.contains(studio)) {
                    items.append(studio);
                }
            }
        }
    } else if (m_widget == MainWidgets::Concerts && m_type == ComboDelegateType::Genres) {
        const auto& concerts = Manager::instance()->concertModel()->concerts();
        for (Concert* concert : concerts) {
            const auto genres = concert->genres();
            for (const QString& genre : genres) {
                if (!genre.isEmpty() && !items.contains(genre)) {
                    items.append(genre);
                }
            }
        }
    } else if (m_widget == MainWidgets::TvShows && m_type == ComboDelegateType::Genres) {
        for (TvShow* show : shows) {
            const auto genres = show->genres();
            for (const QString& genre : genres) {
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
