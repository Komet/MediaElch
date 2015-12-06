#include "FilterWidget.h"
#include "ui_FilterWidget.h"

#include <QGraphicsDropShadowEffect>
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "main/MainWindow.h"
#include "main/Navbar.h"

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
    ui->lineEdit->setType(MyLineEdit::TypeClear);
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_list = new QListWidget();
    m_list->setWindowFlags(Qt::WindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint));
    m_list->setAttribute(Qt::WA_ShowWithoutActivating, true);
    m_list->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_activeWidget = WidgetMovies;

    QPalette palette = m_list->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Highlight));
    palette.setColor(QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
    m_list->setPalette(palette);
    m_list->setStyleSheet(QString("background-color: #ffffff; border: 1px solid #f0f0f0; border-radius: 5px;"));
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    if (Helper::instance()->devicePixelRatio(m_list) == 1) {
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(16);
        effect->setOffset(0);
        effect->setColor(QColor(0, 0, 0, 100));
        m_list->setGraphicsEffect(effect);
    }

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
    m_list->setParent(MainWindow::instance()->centralWidget());
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

        if ((filter->info() == MovieFilters::Path)
            && !m_activeFilters.contains(filter)) {
            filter->setText(tr("Filename contains \"%1\"").arg(text));
            filter->setShortText(text);
        }

        if ((filter->info() == MovieFilters::ImdbId && filter->hasInfo())
            && !m_activeFilters.contains(filter)) {
            filter->setText(tr("IMDB ID \"%1\"").arg(text));
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

        m_list->setFixedHeight(qMin(300, height));
        m_list->setFixedWidth(m_list->sizeHintForColumn(0)+5);
        QPoint globalPos = ui->lineEdit->mapToGlobal(QPoint(ui->lineEdit->paddingLeft(), ui->lineEdit->height()));
        m_list->move(MainWindow::instance()->centralWidget()->mapFromGlobal(globalPos));
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
            if (f->info() == MovieFilters::Title || f->info() == ConcertFilters::Title ||
                    f->info() == TvShowFilters::Title || f->info() == MusicFilters::Title) {
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
    else if (m_activeWidget == WidgetMusic)
        setupMusicFilters();
}

/**
 * @brief Sets up movie filters
 */
void FilterWidget::setupMovieFilters()
{
    QStringList years;
    QStringList genres;
    QStringList certifications;
    QStringList studios;
    QStringList countries;
    QStringList tags;
    QStringList directors;
    QStringList sets;
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        foreach (const QString &genre, movie->genres()) {
            if (!genre.isEmpty() && !genres.contains(genre))
                genres.append(genre);
        }
        foreach (const QString &studio, movie->studios()) {
            if (!studio.isEmpty() && !studios.contains(studio))
                studios.append(studio);
        }
        foreach (const QString &country, movie->countries()) {
            if (!country.isEmpty() && !countries.contains(country))
                countries.append(country);
        }
        foreach (const QString &tag, movie->tags()) {
            if (!tag.isEmpty() && !tags.contains(tag))
                tags.append(tag);
        }
        if (!directors.contains(movie->director()))
            directors.append(movie->director());
        if (movie->released().isValid() && !years.contains(QString("%1").arg(movie->released().year())))
            years.append(QString("%1").arg(movie->released().year()));
        if (!movie->certification().isEmpty() && !certifications.contains(movie->certification()))
            certifications.append(movie->certification());
        if (!movie->set().isEmpty() && !sets.contains(movie->set()))
            sets.append(movie->set());
    }
    qSort(certifications.begin(), certifications.end(), LocaleStringCompare());
    qSort(genres.begin(), genres.end(), LocaleStringCompare());
    qSort(years.begin(), years.end(), LocaleStringCompare());
    qSort(studios.begin(), studios.end(), LocaleStringCompare());
    qSort(countries.begin(), countries.end(), LocaleStringCompare());
    qSort(tags.begin(), tags.end(), LocaleStringCompare());
    qSort(directors.begin(), directors.end(), LocaleStringCompare());
    qSort(sets.begin(), sets.end(), LocaleStringCompare());

    if (m_movieLabelFilters.isEmpty()) {
        QMapIterator<int, QString> it(Helper::instance()->labels());
        while (it.hasNext()) {
            it.next();
            m_movieLabelFilters << new Filter(tr("Label \"%1\"").arg(it.value()), it.value(),
                                              QStringList() << tr("Label") << it.value(), MovieFilters::Label, true, it.key());
        }
    }

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
    foreach (Filter *filter, m_movieStudioFilters) {
        if (!studios.contains(filter->shortText())) {
            m_movieStudioFilters.removeOne(filter);
            delete filter;
        }
    }
    foreach (Filter *filter, m_movieCountryFilters) {
        if (!countries.contains(filter->shortText())) {
            m_movieCountryFilters.removeOne(filter);
            delete filter;
        }
    }
    foreach (Filter *filter, m_movieDirectorFilters) {
        if (!directors.contains(filter->shortText())) {
            m_movieDirectorFilters.removeOne(filter);
            delete filter;
        }
    }
    foreach (Filter *filter, m_movieTagsFilters) {
        if (!tags.contains(filter->shortText())) {
            m_movieTagsFilters.removeOne(filter);
            delete filter;
        }
    }
    foreach (Filter *filter, m_movieSetsFilters) {
        if (!tags.contains(filter->shortText())) {
            m_movieSetsFilters.removeOne(filter);
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

    QList<Filter*> studioFilters;
    // Add new filters
    foreach (const QString &studio, studios) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieStudioFilters) {
            if (filter->shortText() == studio) {
                f = filter;
                break;
            }
        }
        if (f)
            studioFilters << f;
        else
            studioFilters << new Filter(tr("Studio \"%1\"").arg(studio), studio,
                                       QStringList() << tr("Studio") << studio, MovieFilters::Studio, true);
    }
    m_movieStudioFilters = studioFilters;

    QList<Filter*> countryFilters;
    // Add new filters
    foreach (const QString &country, countries) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieCountryFilters) {
            if (filter->shortText() == country) {
                f = filter;
                break;
            }
        }
        if (f)
            countryFilters << f;
        else
            countryFilters << new Filter(tr("Country \"%1\"").arg(country), country,
                                       QStringList() << tr("Country") << country, MovieFilters::Country, true);
    }
    m_movieCountryFilters = countryFilters;

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

    QList<Filter*> setsFilters;
    foreach (const QString &set, sets) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieSetsFilters) {
            if (filter->shortText() == set) {
                f = filter;
                break;
            }
        }
        if (f)
            setsFilters << f;
        else
            setsFilters << new Filter(tr("Set \"%1\"").arg(set), set,
                                               QStringList() << tr("Set") << set, MovieFilters::Set, true);
    }
    m_movieSetsFilters = setsFilters;

    QList<Filter*> tagsFilters;
    foreach (const QString &tag, tags) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieTagsFilters) {
            if (filter->shortText() == tag) {
                f = filter;
                break;
            }
        }
        if (f)
            tagsFilters << f;
        else
            tagsFilters << new Filter(tr("Tag \"%1\"").arg(tag), tag,
                                      QStringList() << tr("Tag") << tag, MovieFilters::Tags, true);
    }
    m_movieTagsFilters = tagsFilters;

    QList<Filter*> directorFilters;
    foreach (const QString &director, directors) {
        Filter *f = 0;
        foreach (Filter *filter, m_movieDirectorFilters) {
            if (filter->shortText() == director) {
                f = filter;
                break;
            }
        }
        if (f)
            directorFilters << f;
        else
            directorFilters << new Filter(tr("Director \"%1\"").arg(director), director,
                                      QStringList() << tr("Director") << director, MovieFilters::Director, true);
    }
    m_movieDirectorFilters = directorFilters;


    QList<Filter*> filters;
    filters << m_movieFilters
            << m_movieGenreFilters
            << m_movieStudioFilters
            << m_movieCountryFilters
            << m_movieYearFilters
            << m_movieCertificationFilters
            << m_movieSetsFilters
            << m_movieTagsFilters
            << m_movieDirectorFilters
            << m_movieLabelFilters;
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

void FilterWidget::setupMusicFilters()
{
    m_filters = m_musicFilters;
}

/**
 * @brief Initially sets up filters
 */
void FilterWidget::initFilters()
{
    m_movieFilters << new Filter(tr("Title"), "", QStringList(), MovieFilters::Title, true);
    m_movieFilters << new Filter(tr("Filename"), "", QStringList(), MovieFilters::Path, true);
    m_movieFilters << new Filter(tr("IMDB ID"), "", QStringList(), MovieFilters::ImdbId, true);

    m_movieFilters << new Filter(tr("Movie has Poster"), tr("Poster"),
                                 QStringList() << tr("Poster"), MovieFilters::Poster, true);

    m_movieFilters << new Filter(tr("Movie has no Poster"), tr("No Poster"),
                                 QStringList() << tr("Poster") << tr("No Poster"), MovieFilters::Poster, false);

    m_movieFilters << new Filter(tr("Movie has Extra Fanarts"), tr("Extra Fanarts"),
                                 QStringList() << tr("Extra Fanarts"), MovieFilters::ExtraFanarts, true);

    m_movieFilters << new Filter(tr("Movie has no Extra Fanarts"), tr("No Extra Fanarts"),
                                 QStringList() << tr("Extra Fanarts") << tr("No Extra Fanarts"), MovieFilters::ExtraFanarts, false);

    m_movieFilters << new Filter(tr("Movie has Backdrop"), tr("Backdrop"),
                                 QStringList() << tr("Backdrop") << tr("Fanart"), MovieFilters::Backdrop, true);

    m_movieFilters << new Filter(tr("Movie has no Backdrop"), tr("No Backdrop"),
                                 QStringList() << tr("Backdrop") << tr("Fanart") << tr("No Backdrop") << tr("No Fanart"), MovieFilters::Backdrop, false);

    m_movieFilters << new Filter(tr("Movie has Logo"), tr("Logo"),
                                 QStringList() << tr("Logo"), MovieFilters::Logo, true);

    m_movieFilters << new Filter(tr("Movie has no Logo"), tr("No Logo"),
                                 QStringList() << tr("Logo") << tr("No Logo"), MovieFilters::Logo, false);

    m_movieFilters << new Filter(tr("Movie has Clear Art"), tr("Clear Art"),
                                 QStringList() << tr("Clear Art"), MovieFilters::ClearArt, true);

    m_movieFilters << new Filter(tr("Movie has no Clear Art"), tr("No Clear Art"),
                                 QStringList() << tr("Clear Art") << tr("No Clear Art"), MovieFilters::ClearArt, false);

    m_movieFilters << new Filter(tr("Movie has Banner"), tr("Banner"),
                                 QStringList() << tr("Banner"), MovieFilters::Banner, true);

    m_movieFilters << new Filter(tr("Movie has no Banner"), tr("No Banner"),
                                 QStringList() << tr("Banner") << tr("No Banner"), MovieFilters::Banner, false);

    m_movieFilters << new Filter(tr("Movie has Thumb"), tr("Thumb"),
                                 QStringList() << tr("Thumb"), MovieFilters::Thumb, true);

    m_movieFilters << new Filter(tr("Movie has no Thumb"), tr("No Thumb"),
                                 QStringList() << tr("Thumb") << tr("No Thumb"), MovieFilters::Thumb, false);

    m_movieFilters << new Filter(tr("Movie has CD Art"), tr("CD Art"),
                                 QStringList() << tr("CD Art"), MovieFilters::CdArt, true);

    m_movieFilters << new Filter(tr("Movie has no CD Art"), tr("No CD Art"),
                                 QStringList() << tr("CD Art") << tr("No CD Art"), MovieFilters::CdArt, false);

    m_movieFilters << new Filter(tr("Movie has Trailer"), tr("Trailer"),
                                 QStringList() << tr("Trailer"), MovieFilters::Trailer, true);

    m_movieFilters << new Filter(tr("Movie has no Trailer"), tr("No Trailer"),
                                 QStringList() << tr("Trailer") << tr("No Trailer"), MovieFilters::Trailer, false);

    m_movieFilters << new Filter(tr("Movie has local Trailer"), tr("Local Trailer"),
                                 QStringList() << tr("Trailer") << tr("Local Trailer"), MovieFilters::LocalTrailer, true);

    m_movieFilters << new Filter(tr("Movie has no local Trailer"), tr("No local Trailer"),
                                 QStringList() << tr("Trailer") << tr("Local Trailer") << tr("No Trailer") << tr("No local Trailer"), MovieFilters::LocalTrailer, false);

    m_movieFilters << new Filter(tr("Movie is Watched"), tr("Watched"),
                                 QStringList() << tr("Watched") << tr("Seen"), MovieFilters::Watched, true);

    m_movieFilters << new Filter(tr("Movie is Unwatched"), tr("Unwatched"),
                                 QStringList() << tr("Watched") << tr("Seen") << tr("Unwatched") << tr("Unseen"), MovieFilters::Watched, false);

    m_movieFilters << new Filter(tr("Movie has no Certification"), tr("No Certification"),
                                 QStringList() << tr("Certification") << tr("No Certification"), MovieFilters::Certification, false);

    m_movieFilters << new Filter(tr("Movie has no Genre"), tr("No Genre"),
                                 QStringList() << tr("Genre") << tr("No Genre"), MovieFilters::Genres, false);

    m_movieFilters << new Filter(tr("Movie has Rating"), tr("Rating"),
                                 QStringList() << tr("Rating"), MovieFilters::Rating, true);

    m_movieFilters << new Filter(tr("Movie has no Rating"), tr("No Rating"),
                                 QStringList() << tr("Rating") << tr("No Rating"), MovieFilters::Rating, false);

    m_movieFilters << new Filter(tr("Stream Details loaded"), tr("Stream Details"),
                                 QStringList() << tr("Stream Details"), MovieFilters::StreamDetails, true);

    m_movieFilters << new Filter(tr("Stream Details not loaded"), tr("No Stream Details"),
                                 QStringList() << tr("Stream Details") << tr("No Stream Details"), MovieFilters::StreamDetails, false);

    m_movieFilters << new Filter(tr("Movie has Actors"), tr("Actors"),
                                 QStringList() << tr("Actors"), MovieFilters::Actors, true);

    m_movieFilters << new Filter(tr("Movie has no Actors"), tr("No Actors"),
                                 QStringList() << tr("Actors") << tr("No Actors"), MovieFilters::Actors, false);

    m_movieFilters << new Filter(tr("Movie has no Studio"), tr("No Studio"),
                                 QStringList() << tr("Studio") << tr("No Studio"), MovieFilters::Studio, false);

    m_movieFilters << new Filter(tr("Movie has no Country"), tr("No Country"),
                                 QStringList() << tr("Country") << tr("No Country"), MovieFilters::Country, false);

    m_movieFilters << new Filter(tr("Movie has no Director"), tr("No Director"),
                                 QStringList() << tr("Director") << tr("No Director"), MovieFilters::Director, false);

    m_movieFilters << new Filter(tr("Movie has no Tags"), tr("No Tags"),
                                 QStringList() << tr("Tags") << tr("No Tags"), MovieFilters::Tags, false);

    m_movieFilters << new Filter(tr("Movie has no IMDB ID"), tr("No IMDB ID"),
                                 QStringList() << tr("IMDB") << tr("No IMDB ID"), MovieFilters::ImdbId, false);

    m_movieFilters << new Filter(tr("Resolution 720p"), "720p",
                                 QStringList() << tr("Resolution") << tr("720p"), MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("Resolution 1080p"), "1080p",
                                 QStringList() << tr("Resolution") << tr("1080p"), MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("Resolution SD"), "SD",
                                 QStringList() << tr("Resolution") << tr("SD"), MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("Format DVD"), "DVD",
                                 QStringList() << tr("Format") << tr("DVD"), MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("BluRay Format"), "BluRay",
                                 QStringList() << tr("Format") << tr("BluRay"), MovieFilters::Quality, true);

    m_movieFilters << new Filter(tr("Channels 2.0"), "2.0",
                                 QStringList() << tr("Audio") << tr("Channels") << "2.0", MovieFilters::AudioChannels, true);
    m_movieFilters << new Filter(tr("Channels 5.1"), "5.1",
                                 QStringList() << tr("Audio") << tr("Channels") << "5.1", MovieFilters::AudioChannels, true);
    m_movieFilters << new Filter(tr("Channels 7.1"), "7.1",
                                 QStringList() << tr("Audio") << tr("Channels") << "2.0", MovieFilters::AudioChannels, true);

    m_movieFilters << new Filter(tr("Audio Quality HD"), "HD Audio",
                                 QStringList() << tr("Audio") << tr("HD Audio") << "True HD" << "DTS HD" << "Dolby", MovieFilters::AudioQuality, true);
    m_movieFilters << new Filter(tr("Audio Quality Normal"), "Normal Audio",
                                 QStringList() << tr("Audio") << tr("Normal Audio") << "DTS" << "AC3" << "Dolby", MovieFilters::AudioQuality, true);
    m_movieFilters << new Filter(tr("Audio Quality SD"), "SD Audio",
                                 QStringList() << tr("Audio") << tr("SD Audio") << "MP3", MovieFilters::AudioQuality, true);

    m_movieFilters << new Filter(tr("Movie has Subtitle"), tr("Subtitle"),
                                 QStringList() << tr("Subtitle"), MovieFilters::HasSubtitle, true);
    m_movieFilters << new Filter(tr("Movie has no Subtitle"), tr("No Subtitle"),
                                 QStringList() << tr("No Subtitle") << tr("Subtitle"), MovieFilters::HasSubtitle, false);

    m_movieFilters << new Filter(tr("Movie has external Subtitle"), tr("External Subtitle"),
                                 QStringList() << tr("Subtitle") << tr("External Subtitle"), MovieFilters::HasExternalSubtitle, true);
    m_movieFilters << new Filter(tr("Movie has no external Subtitle"), tr("No External Subtitle"),
                                 QStringList() << tr("Subtitle") << tr("External Subtitle") << tr("No Subtitle") << tr("No External Subtitle"), MovieFilters::HasExternalSubtitle, false);

    m_tvShowFilters << new Filter(tr("Title"), "", QStringList(), TvShowFilters::Title, true);

    m_concertFilters << new Filter(tr("Title"), "", QStringList(), ConcertFilters::Title, true);

    m_musicFilters << new Filter(tr("Title"), "", QStringList(), MusicFilters::Title, true);
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
        if (m_movieSetsFilters.contains(filter))
            continue;
        if (m_movieStudioFilters.contains(filter))
            continue;
        if (m_movieCountryFilters.contains(filter))
            continue;
        if (m_movieTagsFilters.contains(filter))
            continue;
        if (m_movieDirectorFilters.contains(filter))
            continue;
        if (m_tvShowFilters.contains(filter))
            continue;
        if (m_concertFilters.contains(filter))
            continue;
        if (m_musicFilters.contains(filter))
            continue;
        m_activeFilters.removeOne(filter);
    }

    foreach (Filter *filter, m_activeFilters)
        ui->lineEdit->addFilter(filter);
}
