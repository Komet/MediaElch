#include "ComboDelegate.h"

#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"

/**
 * @brief ComboDelegate::ComboDelegate
 * @param parent
 * @param widget
 * @param type
 */
ComboDelegate::ComboDelegate(QObject *parent, MainWidgets widget, ComboDelegateType type) :
    QItemDelegate(parent),
    m_widget{widget},
    m_type{type}
{
}

/**
 * @brief Sets up a combo box with available genres
 * @param parent
 * @param option
 * @param index
 * @return
 */
QWidget *ComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox *box = new QComboBox(parent);
    box->setEditable(true);
    return box;
}

/**
 * @brief ComboDelegate::setEditorData
 * @param editor
 * @param index
 */
void ComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *box = static_cast<QComboBox*>(editor);
    QStringList items;
    if (m_widget == WidgetMovies && m_type == ComboDelegateGenres) {
        foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
            foreach (const QString &genre, movie->genres()) {
                if (!genre.isEmpty() && !items.contains(genre))
                    items.append(genre);
            }
        }
    } else if (m_widget == WidgetMovies && m_type == ComboDelegateCountries) {
        foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
            foreach (const QString &country, movie->countries()) {
                if (!country.isEmpty() && !items.contains(country))
                    items.append(country);
            }
        }
    } else if (m_widget == WidgetMovies && m_type == ComboDelegateStudios) {
        foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
            foreach (const QString &studio, movie->studios()) {
                if (!studio.isEmpty() && !items.contains(studio))
                    items.append(studio);
            }
        }
    } else if (m_widget == WidgetConcerts && m_type == ComboDelegateGenres) {
        foreach (Concert *concert, Manager::instance()->concertModel()->concerts()) {
            foreach (const QString &genre, concert->genres()) {
                if (!genre.isEmpty() && !items.contains(genre))
                    items.append(genre);
            }
        }
    } else if (m_widget == WidgetTvShows && m_type == ComboDelegateGenres) {
        foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
            foreach (const QString &genre, show->genres()) {
                if (!genre.isEmpty() && !items.contains(genre))
                    items.append(genre);
            }
        }
    } else if (m_widget == WidgetTvShows && m_type == ComboDelegateDirectors) {
        foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
            foreach (TvShowEpisode *episode, show->episodes()) {
                foreach (const QString &director, episode->directors()) {
                    if (!director.isEmpty() && !items.contains(director))
                        items.append(director);
                }
            }
        }
    } else if (m_widget == WidgetTvShows && m_type == ComboDelegateWriters) {
        foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
            foreach (TvShowEpisode *episode, show->episodes()) {
                foreach (const QString &writer, episode->writers()) {
                    if (!writer.isEmpty() && !items.contains(writer))
                        items.append(writer);
                }
            }
        }
    }
    qSort(items.begin(), items.end(), LocaleStringCompare());
    box->addItems(items);
    box->lineEdit()->setText(value);
}

/**
 * @brief ComboDelegate::setModelData
 * @param editor
 * @param model
 * @param index
 */
void ComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *box = static_cast<QComboBox*>(editor);
    QString value = box->currentText();
    model->setData(index, value, Qt::EditRole);
}

/**
 * @brief ComboDelegate::updateEditorGeometry
 * @param editor
 * @param option
 * @param index
 */
void ComboDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

/**
 * @brief ComboDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize ComboDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int width = QItemDelegate::sizeHint(option, index).width();
    return QSize(width, 25);
}
