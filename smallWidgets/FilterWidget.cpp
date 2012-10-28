#include "FilterWidget.h"
#include "ui_FilterWidget.h"

#include "globals/Globals.h"
#include "globals/Manager.h"

/**
 * @brief FilterWidget::FilterWidget
 * @param parent
 */
FilterWidget::FilterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterWidget)
{
    ui->setupUi(this);
    ui->lineEdit->setShowMagnifier(true);
    ui->lineEdit->addAdditionalStyleSheet("QLineEdit { border: 1px solid rgba(0, 0, 0, 100); border-radius: 10px; }");
    ui->lineEdit->setType(MyLineEdit::TypeClear);
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_list = new QListWidget();
    m_list->setWindowFlags(Qt::WindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint));
    m_list->setAttribute(Qt::WA_ShowWithoutActivating);

    m_activeWidget = WidgetMovies;

    QPalette palette = m_list->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Highlight));
    palette.setColor(QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
    m_list->setPalette(palette);
    m_list->setStyleSheet(QString("background-color: transparent; border: 1px solid rgba(255, 255, 255, 200); border-radius: 5px;"));
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(onFilterTextChanged(QString)));
    connect(ui->lineEdit, SIGNAL(keyDown()), this, SLOT(onKeyDown()));
    connect(ui->lineEdit, SIGNAL(keyUp()), this, SLOT(onKeyUp()));
    connect(ui->lineEdit, SIGNAL(focusOut()), m_list, SLOT(hide()));
    connect(ui->lineEdit, SIGNAL(focusIn()), this, SLOT(setupFilters()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(addSelectedFilter()));
    connect(ui->lineEdit, SIGNAL(backspaceInFront()), this, SLOT(removeLastFilter()));
    connect(m_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(addFilterFromItem(QListWidgetItem*)));

    initFilters();
}

/**
 * @brief FilterWidget::~FilterWidget
 */
FilterWidget::~FilterWidget()
{
    delete ui;
}

/**
 * @brief Sets the active main widget
 *        Based on this the filters are set up
 * @param widget
 */
void FilterWidget::setActiveWidget(MainWidgets widget)
{
    storeFilters(m_activeWidget);
    m_activeFilters.clear();
    m_activeWidget = widget;
    ui->lineEdit->clearFilters();
    ui->lineEdit->clear();
    setupFilters();
    loadFilters(m_activeWidget);
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * @brief Selects the next row in the filter list
 */
void FilterWidget::onKeyDown()
{
    if (m_list->isHidden())
        return;
    int row = m_list->currentRow()+1;
    if (row >= m_list->count()-1 || row == 0)
        row = 1;
    m_list->setCurrentRow(row);
}

/**
 * @brief Selects the previous row in the filter list
 */
void FilterWidget::onKeyUp()
{
    if (m_list->isHidden())
        return;
    int row = m_list->currentRow()-1;
    if (row < 1)
        row = m_list->count()-2;
    m_list->setCurrentRow(row);
}

/**
 * @brief Displays the list of available filters
 * @param text Current text in the filter line edit
 */
void FilterWidget::onFilterTextChanged(QString text)
{
    if (text.length() < 3) {
        m_list->hide();
        return;
    }

    int height = 0;
    m_list->clear();
    foreach (Filter *filter, m_filters) {
        if (!filter->accepts(text))
            continue;

        if ((filter->info() == MovieFilters::Title || filter->info() == ConcertFilters::Title || filter->info() == TvShowFilters::Title)
            && !m_activeFilters.contains(filter)) {
            filter->setText(tr("Title contains \"%1\"").arg(text));
            filter->setShortText(text);
        }

        QListWidgetItem *item = new QListWidgetItem(filter->text(), m_list);
        item->setData(Qt::UserRole, QVariant::fromValue(filter));
        item->setBackgroundColor(QColor(255, 255, 255, 200));
        m_list->addItem(item);
        height += m_list->sizeHintForRow(m_list->count()-1);
    }

    if (m_list->count() > 0) {
        QFont font;
        font.setPixelSize(2);

        QListWidgetItem *topItem = new QListWidgetItem("");
        topItem->setFont(font);
        topItem->setBackgroundColor(QColor(255, 255, 255, 200));
        m_list->insertItem(0, topItem);
        height += m_list->sizeHintForRow(0);

        QListWidgetItem *bottomItem = new QListWidgetItem("");
        bottomItem->setBackgroundColor(QColor(255, 255, 255, 200));
        bottomItem->setFont(font);
        m_list->addItem(bottomItem);
        height += m_list->sizeHintForRow(m_list->count()-1);

        m_list->setFixedHeight(height);
        m_list->setFixedWidth(m_list->sizeHintForColumn(0)+5);
        m_list->move(ui->lineEdit->mapToGlobal(QPoint(ui->lineEdit->paddingLeft(), ui->lineEdit->height())));
        m_list->show();
    } else {
        m_list->hide();
    }
}

/**
 * @brief Adds the currently selected filter
 */
void FilterWidget::addSelectedFilter()
{
    m_list->hide();
    Filter *filter = 0;
    if (m_list->currentRow() < 1 || m_list->currentRow() >= m_list->count()-1) {
        if (ui->lineEdit->text().isEmpty())
            return;
        // no entry selected, add the title filter
        foreach (Filter *f, m_filters) {
            if (f->info() == MovieFilters::Title || f->info() == ConcertFilters::Title || f->info() == TvShowFilters::Title) {
                filter = f;
                break;
            }
        }
        if (filter == 0)
            return;
        filter->setText(tr("Title contains \"%1\"").arg(ui->lineEdit->text()));
        filter->setShortText(ui->lineEdit->text());
    } else {
        filter = m_list->currentItem()->data(Qt::UserRole).value<Filter*>();
        if (m_activeFilters.contains(filter))
            return;
    }
    m_activeFilters.append(filter);
    ui->lineEdit->addFilter(filter);

    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * @brief Adds the filter given by the item
 * @param item List widget item to take the filter from
 */
void FilterWidget::addFilterFromItem(QListWidgetItem *item)
{
    if (item->data(Qt::UserRole).value<Filter*>() == 0) {
        return;
    }
    m_list->hide();
    Filter *filter = item->data(Qt::UserRole).value<Filter*>();
    if (m_activeFilters.contains(filter))
        return;
    m_activeFilters.append(filter);
    ui->lineEdit->addFilter(filter);
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * @brief Removes the last filter
 */
void FilterWidget::removeLastFilter()
{
    if (m_activeFilters.count() == 0)
        return;
    m_activeFilters.removeLast();
    ui->lineEdit->removeLastFilter();
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * @brief Sets up filters based on the current widget
 */
void FilterWidget::setupFilters()
{
    if (m_activeWidget == WidgetMovies)
        setupMovieFilters();
    else if (m_activeWidget == WidgetTvShows)
        setupTvShowFilters();
    else if (m_activeWidget == WidgetConcerts)
        setupConcertFilters();
}

/**
 * @brief Sets up movie filters
 */
void FilterWidget::setupMovieFilters()
{
    QStringList years;
    QStringList genres;
    QStringList certifications;
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        foreach (const QString &genre, movie->genres()) {
            if (!genre.isEmpty() && !genres.contains(genre))
                genres.append(genre);
        }
        if (movie->released().isValid() && !years.contains(QString("%1").arg(movie->released().year())))
            years.append(QString("%1").arg(movie->released().year()));
        if (!movie->certification().isEmpty() && !certifications.contains(movie->certification()))
            certifications.append(movie->certification());
    }
    certifications.sort();
    genres.sort();
    years.sort();

    // Clear out all filters which doesn't exist
    foreach (Filter *filter, m_movieGenreFilters) {
        if (!genres.contains(filter->shortText())) {
            m_movieGenreFilters.removeOne(filter);
            delete filter;
        }
    }
    foreach (Filter *filter, m_movieCertificationFilters) {
        if (!certifications.contains(filter->shortText())) {
            m_movieCertificationFilters.removeOne(filter);
            delete filter;
        }
    }
    foreach (Filter *filter, m_movieYearFilters) {
        if (!years.contains(filter->shortText())) {
            m_movieYearFilters.removeOne(filter);
            delete filter;
        }
    }

    QList<Filter*> genreFilters;
    // Add new filters
    foreach (const QString &genre, genres) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieGenreFilters) {
            if (filter->shortText() == genre) {
                f = filter;
                break;
            }
        }
        if (f)
            genreFilters << f;
        else
            genreFilters << new Filter(tr("Genre \"%1\"").arg(genre), genre,
                                       QStringList() << tr("Genre") << genre, MovieFilters::Genres, true);
    }
    m_movieGenreFilters = genreFilters;

    QList<Filter*> yearFilters;
    foreach (const QString &year, years) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieYearFilters) {
            if (filter->shortText() == year) {
                f = filter;
                break;
            }
        }
        if (f)
            yearFilters << f;
        else
            yearFilters << new Filter(tr("Released %1").arg(year), year,
                                      QStringList() << tr("Year") << year, MovieFilters::Released, true);
    }
    m_movieYearFilters = yearFilters;

    QList<Filter*> certificationFilters;
    foreach (const QString &certification, certifications) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieCertificationFilters) {
            if (filter->shortText() == certification) {
                f = filter;
                break;
            }
        }
        if (f)
            certificationFilters << f;
        else
            certificationFilters << new Filter(tr("Certification \"%1\"").arg(certification), certification,
                                               QStringList() << tr("Certification") << certification, MovieFilters::Certification, true);
    }
    m_movieCertificationFilters = certificationFilters;

    QList<Filter*> filters;
    filters << m_movieFilters
            << m_movieGenreFilters
            << m_movieYearFilters
            << m_movieCertificationFilters;
    m_filters = filters;
}

/**
 * @brief Sets up tv show filters
 */
void FilterWidget::setupTvShowFilters()
{
    m_filters = m_tvShowFilters;
}

/**
 * @brief Sets up concert filters
 */
void FilterWidget::setupConcertFilters()
{
    m_filters = m_concertFilters;
}

/**
 * @brief Initially sets up filters
 */
void FilterWidget::initFilters()
{
    m_movieFilters << new Filter(tr("Title"), "", QStringList(), MovieFilters::Title, true);
    m_movieFilters << new Filter(tr("Movie has Poster"), tr("Poster"),
                                 QStringList() << tr("Poster"), MovieFilters::Poster, true);
    m_movieFilters << new Filter(tr("Movie has no Poster"), tr("No Poster"),
                                 QStringList() << tr("Poster"), MovieFilters::Poster, false);
    m_movieFilters << new Filter(tr("Movie has Backdrop"), tr("Backdrop"),
                                 QStringList() << tr("Backdrop") << tr("Fanart"), MovieFilters::Backdrop, true);
    m_movieFilters << new Filter(tr("Movie has no Backdrop"), tr("No Backdrop"),
                                 QStringList() << tr("Backdrop") << tr("Fanart"), MovieFilters::Backdrop, false);
    m_movieFilters << new Filter(tr("Movie has Logo"), tr("Logo"),
                                 QStringList() << tr("Logo"), MovieFilters::Logo, true);
    m_movieFilters << new Filter(tr("Movie has no Logo"), tr("No Logo"),
                                 QStringList() << tr("Logo"), MovieFilters::Logo, false);
    m_movieFilters << new Filter(tr("Movie has Clear Art"), tr("Clear Art"),
                                 QStringList() << tr("Clear Art"), MovieFilters::ClearArt, true);
    m_movieFilters << new Filter(tr("Movie has no Clear Art"), tr("No Clear Art"),
                                 QStringList() << tr("Clear Art"), MovieFilters::ClearArt, false);
    m_movieFilters << new Filter(tr("Movie has CD Art"), tr("CD Art"),
                                 QStringList() << tr("CD Art"), MovieFilters::CdArt, true);
    m_movieFilters << new Filter(tr("Movie has no CD Art"), tr("No CD Art"),
                                 QStringList() << tr("CD Art"), MovieFilters::CdArt, false);
    m_movieFilters << new Filter(tr("Movie has Trailer"), tr("Trailer"),
                                 QStringList() << tr("Trailer"), MovieFilters::Trailer, true);
    m_movieFilters << new Filter(tr("Movie has no Trailer"), tr("No Trailer"),
                                 QStringList() << tr("Trailer"), MovieFilters::Trailer, false);
    m_movieFilters << new Filter(tr("Movie is Watched"), tr("Watched"),
                                 QStringList() << tr("Watched") << tr("Seen"), MovieFilters::Watched, false);
    m_movieFilters << new Filter(tr("Movie is Unwatched"), tr("Unwatched"),
                                 QStringList() << tr("Watched") << tr("Seen") << tr("Unwatched") << tr("Unseen"), MovieFilters::Watched, false);
    m_movieFilters << new Filter(tr("Movie has no Certification"), tr("No Certification"),
                                 QStringList() << tr("Certification"), MovieFilters::Certification, false);
    m_movieFilters << new Filter(tr("Movie has no Genre"), tr("No Genre"),
                                 QStringList() << tr("Genre"), MovieFilters::Genres, false);

    m_tvShowFilters << new Filter(tr("Title"), "", QStringList(), TvShowFilters::Title, true);

    m_concertFilters << new Filter(tr("Title"), "", QStringList(), ConcertFilters::Title, true);
}

/**
 * @brief Store filters for widget
 * @param widget Active widget
 */
void FilterWidget::storeFilters(MainWidgets widget)
{
    if (m_storedFilters.contains(widget)) {
        m_storedFilters[widget] = m_activeFilters;
    } else {
        m_storedFilters.insert(widget, m_activeFilters);
    }
}

/**
 * @brief Load filters, stored before
 * @param widget Active widget
 */
void FilterWidget::loadFilters(MainWidgets widget)
{
    if (!m_storedFilters.contains(widget))
        return;
    m_activeFilters = m_storedFilters[widget];

    // clean up not existent filters
    foreach (Filter *filter, m_activeFilters) {
        if (m_movieFilters.contains(filter))
            continue;
        if (m_movieGenreFilters.contains(filter))
            continue;
        if (m_movieCertificationFilters.contains(filter))
            continue;
        if (m_movieYearFilters.contains(filter))
            continue;
        if (m_tvShowFilters.contains(filter))
            continue;
        if (m_concertFilters.contains(filter))
            continue;
        m_activeFilters.removeOne(filter);
    }

    foreach (Filter *filter, m_activeFilters)
        ui->lineEdit->addFilter(filter);
}
