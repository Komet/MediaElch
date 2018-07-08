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
FilterWidget::FilterWidget(QWidget *parent) : QWidget(parent), ui(new Ui::FilterWidget)
{
    ui->setupUi(this);
    ui->lineEdit->setShowMagnifier(true);
    ui->lineEdit->setType(MyLineEdit::TypeClear);
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_list = new QListWidget();
    m_list->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_list->setAttribute(Qt::WA_ShowWithoutActivating, true);
    m_list->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_activeWidget = MainWidgets::Movies;

    QPalette palette = m_list->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Highlight));
    palette.setColor(QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
    m_list->setPalette(palette);
    m_list->setStyleSheet(QStringLiteral("background-color: #ffffff; border: 1px solid #f0f0f0; border-radius: 5px;"));
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    const qreal pixelRatio = Helper::instance()->devicePixelRatio(m_list);
    if (pixelRatio >= 0.95 && pixelRatio <= 1.05) {
        // Pixel ratio is 1
        auto effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(16);
        effect->setOffset(0);
        effect->setColor(QColor(0, 0, 0, 100));
        m_list->setGraphicsEffect(effect);
    }

    connect(ui->lineEdit, &QLineEdit::textEdited, this, &FilterWidget::onFilterTextChanged);
    connect(ui->lineEdit, &MyLineEdit::keyDown, this, &FilterWidget::onKeyDown);
    connect(ui->lineEdit, &MyLineEdit::keyUp, this, &FilterWidget::onKeyUp);
    connect(ui->lineEdit, &MyLineEdit::focusOut, m_list, &QWidget::hide);
    connect(ui->lineEdit, &MyLineEdit::focusIn, this, &FilterWidget::setupFilters);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &FilterWidget::addSelectedFilter);
    connect(ui->lineEdit, &MyLineEdit::backspaceInFront, this, &FilterWidget::removeLastFilter);
    connect(m_list, &QListWidget::itemClicked, this, &FilterWidget::addFilterFromItem);

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
    if (m_list->isHidden()) {
        return;
    }
    int row = m_list->currentRow() + 1;
    if (row >= m_list->count() - 1 || row == 0) {
        row = 1;
    }
    m_list->setCurrentRow(row);
}

/**
 * @brief Selects the previous row in the filter list
 */
void FilterWidget::onKeyUp()
{
    if (m_list->isHidden()) {
        return;
    }
    int row = m_list->currentRow() - 1;
    if (row < 1) {
        row = m_list->count() - 2;
    }
    m_list->setCurrentRow(row);
}

/**
 * @brief Displays the list of available filters
 * @param text Current text in the filter line edit
 */
void FilterWidget::onFilterTextChanged(QString text)
{
    m_list->setParent(MainWindow::instance()->centralWidget());
    if (text.length() < 2) {
        m_list->hide();
        return;
    }

    int height = 0;
    m_list->clear();
    for (auto filter : m_filters) {
        if (!filter->accepts(text) || m_activeFilters.contains(filter)) {
            // Each filter can only be applied once.
            continue;
        }

        if (filter->isInfo(MovieFilters::Title) || filter->isInfo(ConcertFilters::Title)
            || filter->isInfo(TvShowFilters::Title) || filter->isInfo(MusicFilters::Title)) {
            filter->setText(tr("Title contains \"%1\"").arg(text));
            filter->setShortText(text);
        }

        if (filter->isInfo(MovieFilters::Path)) {
            filter->setText(tr("Filename contains \"%1\"").arg(text));
            filter->setShortText(text);
        }

        if (filter->isInfo(MovieFilters::ImdbId) && filter->hasInfo()) {
            filter->setText(tr("IMDB ID \"%1\"").arg(text));
            filter->setShortText(text);
        }

        QListWidgetItem *item = new QListWidgetItem(filter->text(), m_list);
        item->setData(Qt::UserRole, QVariant::fromValue(filter));
        item->setBackgroundColor(QColor(255, 255, 255, 200));
        m_list->addItem(item);
        height += m_list->sizeHintForRow(m_list->count() - 1);
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
        height += m_list->sizeHintForRow(m_list->count() - 1);

        m_list->setFixedHeight(qMin(300, height));
        m_list->setFixedWidth(m_list->sizeHintForColumn(0) + 5);
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
    Filter *filter = nullptr;
    if (m_list->currentRow() < 1 || m_list->currentRow() >= m_list->count() - 1) {
        if (ui->lineEdit->text().isEmpty()) {
            return;
        }
        // no entry selected, add the title filter
        for (Filter *f : m_filters) {
            if ((f->isInfo(MovieFilters::Title) || f->isInfo(ConcertFilters::Title) || f->isInfo(TvShowFilters::Title)
                    || f->isInfo(MusicFilters::Title))
                && !m_activeFilters.contains(f)) {
                filter = f;
                break;
            }
        }
        if (filter == nullptr) {
            return;
        }
        filter->setText(tr("Title contains \"%1\"").arg(ui->lineEdit->text()));
        filter->setShortText(ui->lineEdit->text());
    } else {
        filter = m_list->currentItem()->data(Qt::UserRole).value<Filter *>();
        if (m_activeFilters.contains(filter)) {
            return;
        }
    }

    if (filter != nullptr) {
        m_activeFilters.append(filter);
        ui->lineEdit->addFilter(filter);
        emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
    }
}

/**
 * @brief Adds the filter given by the item
 * @param item List widget item to take the filter from
 */
void FilterWidget::addFilterFromItem(QListWidgetItem *item)
{
    if (item->data(Qt::UserRole).value<Filter *>() == nullptr) {
        return;
    }
    m_list->hide();
    auto filter = item->data(Qt::UserRole).value<Filter *>();
    if (m_activeFilters.contains(filter)) {
        return;
    }
    m_activeFilters.append(filter);
    ui->lineEdit->addFilter(filter);
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * @brief Removes the last filter
 */
void FilterWidget::removeLastFilter()
{
    if (m_activeFilters.count() == 0) {
        return;
    }
    m_activeFilters.removeLast();
    ui->lineEdit->removeLastFilter();
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * @brief Sets up filters based on the current widget
 */
void FilterWidget::setupFilters()
{
    if (m_activeWidget == MainWidgets::Movies) {
        setupMovieFilters();
    } else if (m_activeWidget == MainWidgets::TvShows) {
        setupTvShowFilters();
    } else if (m_activeWidget == MainWidgets::Concerts) {
        setupConcertFilters();
    } else if (m_activeWidget == MainWidgets::Music) {
        setupMusicFilters();
    }
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
    QStringList videocodecs;
    QStringList sets;

    const auto copyNotEmptyUnique = [](const QStringList &from, QStringList &to) {
        for (const QString &str : from) {
            if (!str.isEmpty() && !to.contains(str)) {
                to.append(str);
            }
        }
    };

    for (Movie *movie : Manager::instance()->movieModel()->movies()) {
        copyNotEmptyUnique(movie->genres(), genres);
        copyNotEmptyUnique(movie->studios(), studios);
        copyNotEmptyUnique(movie->countries(), countries);
        copyNotEmptyUnique(movie->tags(), tags);

        if (!directors.contains(movie->director())) {
            directors.append(movie->director());
        }
        if (!videocodecs.contains(movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::Codec))) {
            videocodecs.append(movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::Codec));
        }
        if (movie->released().isValid() && !years.contains(QString::number(movie->released().year()))) {
            years.append(QString::number(movie->released().year()));
        }
        if (!movie->certification().isEmpty() && !certifications.contains(movie->certification())) {
            certifications.append(movie->certification());
        }
        if (!movie->set().isEmpty() && !sets.contains(movie->set())) {
            sets.append(movie->set());
        }
    }
    qSort(certifications.begin(), certifications.end(), LocaleStringCompare());
    qSort(genres.begin(), genres.end(), LocaleStringCompare());
    qSort(years.begin(), years.end(), LocaleStringCompare());
    qSort(studios.begin(), studios.end(), LocaleStringCompare());
    qSort(countries.begin(), countries.end(), LocaleStringCompare());
    qSort(tags.begin(), tags.end(), LocaleStringCompare());
    qSort(directors.begin(), directors.end(), LocaleStringCompare());
    qSort(videocodecs.begin(), videocodecs.end(), LocaleStringCompare());
    qSort(sets.begin(), sets.end(), LocaleStringCompare());

    if (m_movieLabelFilters.isEmpty()) {
        QMapIterator<int, QString> it(Helper::instance()->labels());
        while (it.hasNext()) {
            it.next();
            m_movieLabelFilters << new Filter(tr("Label \"%1\"").arg(it.value()),
                it.value(),
                QStringList() << tr("Label") << it.value(),
                MovieFilters::Label,
                true,
                it.key());
        }
    }

    // Clear out all filters which don't exist
    const auto removeFiltersNotInList = [](QList<Filter *> &filters, const QStringList &list) {
        for (Filter *filter : filters) {
            if (!list.contains(filter->shortText())) {
                filters.removeOne(filter);
                delete filter;
            }
        }
    };
    removeFiltersNotInList(m_movieGenreFilters, genres);
    removeFiltersNotInList(m_movieCertificationFilters, certifications);
    removeFiltersNotInList(m_movieYearFilters, years);
    removeFiltersNotInList(m_movieStudioFilters, studios);
    removeFiltersNotInList(m_movieCountryFilters, countries);
    removeFiltersNotInList(m_movieDirectorFilters, directors);
    removeFiltersNotInList(m_movieVideoCodecFilters, videocodecs);
    removeFiltersNotInList(m_movieTagsFilters, tags);
    removeFiltersNotInList(m_movieSetsFilters, sets);

    const auto setNewFilters = [](QList<Filter *> &existingFilters,
                                   const QStringList &filtersToApply,
                                   QString filterTypeName,
                                   MovieFilters infoType) {
        QList<Filter *> newFilters;
        for (const QString &filterName : filtersToApply) {
            Filter *f = nullptr;
            for (Filter *filter : existingFilters) {
                if (filter->shortText() == filterName) {
                    f = filter;
                    break;
                }
            }
            if (f) {
                newFilters << f;
            } else {
                newFilters << new Filter(QStringLiteral("%1 \"%2\"").arg(filterName),
                    filterName,
                    QStringList() << filterTypeName << filterName,
                    infoType,
                    true);
            }
        }
        existingFilters = newFilters;
    };

    setNewFilters(m_movieGenreFilters, genres, tr("Genre"), MovieFilters::Genres);
    setNewFilters(m_movieStudioFilters, studios, tr("Studio"), MovieFilters::Studio);
    setNewFilters(m_movieCountryFilters, countries, tr("Country"), MovieFilters::Country);
    setNewFilters(m_movieCertificationFilters, certifications, tr("Certification"), MovieFilters::Certification);
    setNewFilters(m_movieSetsFilters, sets, tr("Set"), MovieFilters::Set);
    setNewFilters(m_movieTagsFilters, tags, tr("Tag"), MovieFilters::Tags);
    setNewFilters(m_movieDirectorFilters, directors, tr("Director"), MovieFilters::Director);
    setNewFilters(m_movieVideoCodecFilters, videocodecs, tr("Video codec"), MovieFilters::VideoCodec);

    QList<Filter *> yearFilters;
    for (const QString &year : years) {
        Filter *f = nullptr;
        foreach (Filter *filter, m_movieYearFilters) {
            if (filter->shortText() == year) {
                f = filter;
                break;
            }
        }
        if (f) {
            yearFilters << f;
        } else {
            yearFilters << new Filter(
                tr("Released %1").arg(year), year, QStringList() << tr("Year") << year, MovieFilters::Released, true);
        }
    }
    m_movieYearFilters = yearFilters;

    QList<Filter *> filters;
    filters << m_movieFilters              //
            << m_movieGenreFilters         //
            << m_movieStudioFilters        //
            << m_movieCountryFilters       //
            << m_movieYearFilters          //
            << m_movieCertificationFilters //
            << m_movieSetsFilters          //
            << m_movieTagsFilters          //
            << m_movieDirectorFilters      //
            << m_movieVideoCodecFilters    //
            << m_movieLabelFilters;
    m_filters = filters;
}

void FilterWidget::setupTvShowFilters()
{
    m_filters = m_tvShowFilters;
}

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
    // clang-format off
    m_movieFilters << new Filter(tr("Title"),                "",               QStringList(),                  MovieFilters::Title,  true);
    m_movieFilters << new Filter(tr("Filename"),             "",               QStringList(),                  MovieFilters::Path,   true);
    m_movieFilters << new Filter(tr("IMDB ID"),              "",               QStringList(),                  MovieFilters::ImdbId, true);
    m_movieFilters << new Filter(tr("Movie has no IMDB ID"), tr("No IMDB ID"), {tr("IMDB"), tr("No IMDB ID")}, MovieFilters::ImdbId, false);

    // Information
    m_movieFilters << new Filter(tr("Movie has no Studio"),        tr("No Studio"),        {tr("Studio"),        tr("No Studio")},        MovieFilters::Studio,        false);
    m_movieFilters << new Filter(tr("Movie has no Country"),       tr("No Country"),       {tr("Country"),       tr("No Country")},       MovieFilters::Country,       false);
    m_movieFilters << new Filter(tr("Movie has no Director"),      tr("No Director"),      {tr("Director"),      tr("No Director")},      MovieFilters::Director,      false);
    m_movieFilters << new Filter(tr("Movie has no Genre"),         tr("No Genre"),         {tr("Genre"),         tr("No Genre")},         MovieFilters::Genres,        false);
    m_movieFilters << new Filter(tr("Movie has no Tags"),          tr("No Tags"),          {tr("Tags"),          tr("No Tags")},          MovieFilters::Tags,          false);
    m_movieFilters << new Filter(tr("Movie has no Certification"), tr("No Certification"), {tr("Certification"), tr("No Certification")}, MovieFilters::Certification, false);

    // Watched/Rating
    m_movieFilters << new Filter(tr("Movie is Watched"),    tr("Watched"),   {tr("Watched"), tr("Seen")},                                MovieFilters::Watched, true);
    m_movieFilters << new Filter(tr("Movie is Unwatched"),  tr("Unwatched"), {tr("Watched"), tr("Seen"), tr("Unwatched"), tr("Unseen")}, MovieFilters::Watched, false);
    m_movieFilters << new Filter(tr("Movie has Rating"),    tr("Rating"),    {tr("Rating")},                                             MovieFilters::Rating,  true);
    m_movieFilters << new Filter(tr("Movie has no Rating"), tr("No Rating"), {tr("Rating"), tr("No Rating")},                            MovieFilters::Rating,  false);

    // Actors
    m_movieFilters << new Filter(tr("Movie has Actors"),    tr("Actors"),    {"Actors"},                      MovieFilters::Actors, true);
    m_movieFilters << new Filter(tr("Movie has no Actors"), tr("No Actors"), {tr("Actors"), tr("No Actors")}, MovieFilters::Actors, false);

    // Streamdetails
    m_movieFilters << new Filter(tr("Stream Details loaded"),            tr("Stream Details"),    {"Stream Details"},                              MovieFilters::StreamDetails, true);
    m_movieFilters << new Filter(tr("Stream Details not loaded"),        tr("No Stream Details"), {tr("Stream Details"), tr("No Stream Details")}, MovieFilters::StreamDetails, false);
    m_movieFilters << new Filter(tr("No information about video codec"), tr("No video codec"),    {tr("Video codec"), tr("No video codec")},       MovieFilters::VideoCodec,    false);

    // Images
    m_movieFilters << new Filter(tr("Movie has Poster"),           tr("Poster"),           {tr("Poster")},                                                     MovieFilters::Poster,       true);
    m_movieFilters << new Filter(tr("Movie has no Poster"),        tr("No Poster"),        {tr("Poster"), tr("No Poster")},                                    MovieFilters::Poster,       false);
    m_movieFilters << new Filter(tr("Movie has Extra Fanarts"),    tr("Extra Fanarts"),    {tr("Extra Fanarts")},                                              MovieFilters::ExtraFanarts, true);
    m_movieFilters << new Filter(tr("Movie has no Extra Fanarts"), tr("No Extra Fanarts"), {tr("Extra Fanarts"), tr("No Extra Fanarts")},                      MovieFilters::ExtraFanarts, false);
    m_movieFilters << new Filter(tr("Movie has Backdrop"),         tr("Backdrop"),         {tr("Backdrop"), tr("Fanart")},                                     MovieFilters::Backdrop,     true);
    m_movieFilters << new Filter(tr("Movie has no Backdrop"),      tr("No Backdrop"),      {tr("Backdrop"), tr("Fanart"), tr("No Backdrop"), tr("No Fanart")}, MovieFilters::Backdrop,     false);
    m_movieFilters << new Filter(tr("Movie has Logo"),             tr("Logo"),             {tr("Logo")},                                                       MovieFilters::Logo,         true);
    m_movieFilters << new Filter(tr("Movie has no Logo"),          tr("No Logo"),          {tr("Logo"), tr("No Logo")},                                        MovieFilters::Logo,         false);
    m_movieFilters << new Filter(tr("Movie has Clear Art"),        tr("Clear Art"),        {tr("Clear Art")},                                                  MovieFilters::ClearArt,     true);
    m_movieFilters << new Filter(tr("Movie has no Clear Art"),     tr("No Clear Art"),     {tr("Clear Art"), tr("No Clear Art")},                              MovieFilters::ClearArt,     false);
    m_movieFilters << new Filter(tr("Movie has Banner"),           tr("Banner"),           {tr("Banner")},                                                     MovieFilters::Banner,       true);
    m_movieFilters << new Filter(tr("Movie has no Banner"),        tr("No Banner"),        {tr("Banner"), tr("No Banner")},                                    MovieFilters::Banner,       false);
    m_movieFilters << new Filter(tr("Movie has Thumb"),            tr("Thumb"),            {"Thumb"},                                                          MovieFilters::Thumb,        true);
    m_movieFilters << new Filter(tr("Movie has no Thumb"),         tr("No Thumb"),         {tr("Thumb"), tr("No Thumb")},                                      MovieFilters::Thumb,        false);
    m_movieFilters << new Filter(tr("Movie has CD Art"),           tr("CD Art"),           {tr("CD Art")},                                                     MovieFilters::CdArt,        true);
    m_movieFilters << new Filter(tr("Movie has no CD Art"),        tr("No CD Art"),        {tr("CD Art"), tr("No CD Art")},                                    MovieFilters::CdArt,        false);

    // Trailer
    m_movieFilters << new Filter(tr("Movie has Trailer"),          tr("Trailer"),          {tr("Trailer")},                                                                MovieFilters::Trailer,     true);
    m_movieFilters << new Filter(tr("Movie has no Trailer"),       tr("No Trailer"),       {tr("Trailer"), tr("No Trailer")},                                              MovieFilters::Trailer,     false);
    m_movieFilters << new Filter(tr("Movie has local Trailer"),    tr("Local Trailer"),    {tr("Trailer"), tr("Local Trailer")},                                           MovieFilters::LocalTrailer, true);
    m_movieFilters << new Filter(tr("Movie has no local Trailer"), tr("No local Trailer"), {tr("Trailer"), tr("Local Trailer"), tr("No Trailer"), tr("No local Trailer")}, MovieFilters::LocalTrailer, false);

    // Quality
    m_movieFilters << new Filter(tr("Resolution 720p"),  "720p",   {tr("Resolution"), tr("720p")},  MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("Resolution 1080p"), "1080p",  {tr("Resolution"), tr("1080p")}, MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("Resolution 2160p"), "2160p",  {tr("Resolution"), tr("2160p")}, MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("Resolution SD"),    "SD",     {tr("Resolution"), tr("SD")},    MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("Format DVD"),       "DVD",    {tr("Format"), tr("DVD")},       MovieFilters::Quality, true);
    m_movieFilters << new Filter(tr("BluRay Format"),    "BluRay", {tr("Format"), tr("BluRay")},    MovieFilters::Quality, true);

    // Audiochannels
    m_movieFilters << new Filter(tr("Channels 2.0"),         "2.0",          {tr("Audio"), tr("Channels"), "2.0"},                        MovieFilters::AudioChannels, true);
    m_movieFilters << new Filter(tr("Channels 5.1"),         "5.1",          {tr("Audio"), tr("Channels"), "5.1"},                        MovieFilters::AudioChannels, true);
    m_movieFilters << new Filter(tr("Channels 7.1"),         "7.1",          {tr("Audio"), tr("Channels"), "2.0"},                        MovieFilters::AudioChannels, true);
    m_movieFilters << new Filter(tr("Audio Quality HD"),     "HD Audio",     {tr("Audio"), tr("HD Audio"), "True HD", "DTS HD", "Dolby"}, MovieFilters::AudioQuality,  true);
    m_movieFilters << new Filter(tr("Audio Quality Normal"), "Normal Audio", {tr("Audio"), tr("Normal Audio"), "DTS", "AC3", "Dolby"},    MovieFilters::AudioQuality,  true);
    m_movieFilters << new Filter(tr("Audio Quality SD"),     "SD Audio",     {tr("Audio"), tr("SD Audio"), "MP3"},                        MovieFilters::AudioQuality,  true);

    // Subtitle
    m_movieFilters << new Filter(tr("Movie has Subtitle"),               tr("Subtitle"),             {tr("Subtitle")},                                                                         MovieFilters::HasSubtitle,         true);
    m_movieFilters << new Filter(tr("Movie has no Subtitle"),            tr("No Subtitle"),          {tr("No Subtitle"), tr("Subtitle")},                                                      MovieFilters::HasSubtitle,         false);
    m_movieFilters << new Filter(tr("Movie has external Subtitle"),      tr("External Subtitle"),    {tr("Subtitle"), tr("External Subtitle")},                                                MovieFilters::HasExternalSubtitle, true);
    m_movieFilters << new Filter(tr("Movie has no external Subtitle"),   tr("No External Subtitle"), {tr("Subtitle"), tr("External Subtitle"), tr("No Subtitle"), tr("No External Subtitle")}, MovieFilters::HasExternalSubtitle, false);
    // clang-format on

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
    if (!m_storedFilters.contains(widget)) {
        return;
    }
    m_activeFilters = m_storedFilters[widget];

    QVector<QList<Filter *> *> allFilterLists{&m_movieFilters,
        &m_movieGenreFilters,
        &m_movieCertificationFilters,
        &m_movieYearFilters,
        &m_movieSetsFilters,
        &m_movieStudioFilters,
        &m_movieCountryFilters,
        &m_movieTagsFilters,
        &m_movieDirectorFilters,
        &m_movieVideoCodecFilters,
        &m_tvShowFilters,
        &m_concertFilters,
        &m_musicFilters};

    // clean up non-existent filters
    for (Filter *filter : m_activeFilters) {
        bool filterNotExistent = true;
        for (const auto &list : allFilterLists) {
            if (list->contains(filter)) {
                filterNotExistent = false;
                break;
            }
        }
        if (filterNotExistent) {
            m_activeFilters.removeOne(filter);
        }
    }

    for (Filter *filter : m_activeFilters) {
        ui->lineEdit->addFilter(filter);
    }
}
