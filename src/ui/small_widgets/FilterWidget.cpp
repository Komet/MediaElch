#include "FilterWidget.h"
#include "ui_FilterWidget.h"

#include <QGraphicsDropShadowEffect>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "ui/main/MainWindow.h"
#include "ui/main/Navbar.h"

FilterWidget::FilterWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::FilterWidget), m_activeWidget{MainWidgets::Movies}
{
    ui->setupUi(this);
    ui->lineEdit->setShowMagnifier(true);
    ui->lineEdit->setType(MyLineEdit::TypeClear);
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

    setupFilterListUi();

    // clang-format off
    connect(ui->lineEdit, &QLineEdit::textEdited,        this,   &FilterWidget::onFilterTextChanged);
    connect(ui->lineEdit, &MyLineEdit::keyDown,          this,   &FilterWidget::onKeyDown);
    connect(ui->lineEdit, &MyLineEdit::keyUp,            this,   &FilterWidget::onKeyUp);
    connect(ui->lineEdit, &MyLineEdit::focusIn,          this,   &FilterWidget::setupFilters);
    connect(ui->lineEdit, &MyLineEdit::focusOut,         m_list, &QWidget::hide);
    connect(ui->lineEdit, &QLineEdit::returnPressed,     this,   &FilterWidget::addSelectedFilter);
    connect(ui->lineEdit, &MyLineEdit::backspaceInFront, this,   &FilterWidget::removeLastFilter);
    connect(ui->lineEdit, &MyLineEdit::clearClicked,     this,   &FilterWidget::clearFilters);
    connect(m_list,       &QListWidget::itemClicked,     this,   &FilterWidget::addFilterFromItem);
    // clang-format on

    initAvailableFilters();
}

FilterWidget::~FilterWidget()
{
    // Delete original filters
    // todo: There are still memory leaks when using setupMovieFilters()
    for (Filter* filter : m_availableMovieFilters) {
        delete filter;
    }
    for (Filter* filter : m_availableTvShowFilters) {
        delete filter;
    }
    for (Filter* filter : m_availableConcertFilters) {
        delete filter;
    }
    for (Filter* filter : m_availableMusicFilters) {
        delete filter;
    }
    delete ui;
}

void FilterWidget::setActiveMainWidget(MainWidgets widget)
{
    storeFilters(m_activeWidget);
    ui->lineEdit->clearFilters();
    ui->lineEdit->clear();
    m_activeWidget = widget;

    m_activeFilters.clear();
    setupFilters();
    loadFilters(m_activeWidget);

    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * \brief Selects the next row in the filter list
 */
void FilterWidget::onKeyDown()
{
    if (m_list->isHidden()) {
        return;
    }
    int row = m_list->currentRow() + 1;
    // Last row must not be selected as it is empty.
    if (row > m_list->count() - 2 || row == 0) {
        row = 1;
    }
    m_list->setCurrentRow(row);
}

/**
 * \brief Selects the previous row in the filter list
 */
void FilterWidget::onKeyUp()
{
    if (m_list->isHidden()) {
        return;
    }
    int row = m_list->currentRow() - 1;
    // First row must not be selected as it is empty.
    if (row < 1) {
        row = m_list->count() - 2;
    }
    m_list->setCurrentRow(row);
}

/**
 * \brief Displays the list of available filters
 * \param text Current text in the filter line edit
 */
void FilterWidget::onFilterTextChanged(QString text)
{
    m_list->setParent(MainWindow::instance()->centralWidget());
    if (text.length() < 2) {
        m_list->hide();
        return;
    }

    m_list->clear();
    for (auto filter : m_availableFilters) {
        if (!filter->accepts(text) || m_activeFilters.contains(filter)) {
            // Each filter can only be applied once.
            continue;
        }

        if (filter->isInfo(MovieFilters::Title) || filter->isInfo(ConcertFilters::Title)
            || filter->isInfo(TvShowFilters::Title) || filter->isInfo(MusicFilters::Title)) {
            filter->setText(tr("Title contains \"%1\"").arg(text));
            filter->setShortText(text);
        }

        if (filter->isInfo(MovieFilters::OriginalTitle)) {
            filter->setText(tr("Original Title contains \"%1\"").arg(text));
            filter->setShortText(text);
        }

        if (filter->isInfo(MovieFilters::Path)) {
            filter->setText(tr("Filename contains \"%1\"").arg(text));
            filter->setShortText(text);
        }

        if (filter->isInfo(MovieFilters::ImdbId) && filter->hasInfo()) {
            filter->setText(tr("IMDb ID \"%1\"").arg(text));
            filter->setShortText(text);
        }

        if (filter->isInfo(MovieFilters::TmdbId) && filter->hasInfo()) {
            filter->setText(tr("TMDb ID \"%1\"").arg(text));
            filter->setShortText(text);
        }

        auto* item = new QListWidgetItem(filter->text(), m_list);
        item->setData(Qt::UserRole, QVariant::fromValue(filter));
        item->setBackground(QColor(255, 255, 255, 200));
        m_list->addItem(item);
    }

    if (m_list->count() == 0) {
        m_list->hide();
        return;
    }

    int listHeight = 0;
    for (int i = 0; i < m_list->count(); ++i) {
        listHeight += m_list->sizeHintForRow(i);
    }

    // By adding a top and bottom item we can avoid scrolling
    // down the list widget.
    QFont font;
    font.setPixelSize(2);

    auto* topItem = new QListWidgetItem("");
    topItem->setFont(font);
    topItem->setBackground(QColor(255, 255, 255, 200));
    m_list->insertItem(0, topItem);
    listHeight += m_list->sizeHintForRow(0);

    auto* bottomItem = new QListWidgetItem("");
    bottomItem->setBackground(QColor(255, 255, 255, 200));
    bottomItem->setFont(font);
    m_list->addItem(bottomItem);
    listHeight += m_list->sizeHintForRow(m_list->count() - 1);

    // Set width, height and position of the list
    m_list->setFixedHeight(qMin(300, listHeight));
    m_list->setFixedWidth(m_list->sizeHintForColumn(0) + 5);
    QPoint globalPos = ui->lineEdit->mapToGlobal(QPoint(ui->lineEdit->paddingLeft(), ui->lineEdit->height()));
    m_list->move(MainWindow::instance()->centralWidget()->mapFromGlobal(globalPos));
    m_list->show();
}

/**
 * \brief Adds the currently selected filter
 */
void FilterWidget::addSelectedFilter()
{
    if (m_list->currentRow() > 0 && m_list->currentRow() < m_list->count()) {
        addFilterFromItem(m_list->currentItem());
        return;
    }

    // no entry selected, add the title filter

    m_list->hide();

    if (ui->lineEdit->text().isEmpty()) {
        return;
    }

    Filter* titleFilter = nullptr;
    for (Filter* f : m_availableFilters) {
        if ((f->isInfo(MovieFilters::Title) || f->isInfo(ConcertFilters::Title) || f->isInfo(TvShowFilters::Title)
                || f->isInfo(MusicFilters::Title))
            && !m_activeFilters.contains(f)) {
            titleFilter = f;
            break;
        }
    }

    if (titleFilter == nullptr) {
        return;
    }

    titleFilter->setText(tr("Title contains \"%1\"").arg(ui->lineEdit->text()));
    titleFilter->setShortText(ui->lineEdit->text());

    m_activeFilters.append(titleFilter);
    ui->lineEdit->addFilter(titleFilter);
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * \brief Adds the filter given by the item
 * \param item List widget item to take the filter from
 */
void FilterWidget::addFilterFromItem(QListWidgetItem* item)
{
    m_list->hide();
    auto* filter = item->data(Qt::UserRole).value<Filter*>();
    if (filter == nullptr) {
        return;
    }
    if (m_activeFilters.contains(filter)) {
        return;
    }
    m_activeFilters.append(filter);
    ui->lineEdit->addFilter(filter);
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

/**
 * \brief Removes the last filter
 */
void FilterWidget::removeLastFilter()
{
    if (m_activeFilters.isEmpty()) {
        return;
    }
    m_activeFilters.removeLast();
    ui->lineEdit->removeLastFilter();
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
}

void FilterWidget::clearFilters()
{
    m_activeFilters.clear();
    ui->lineEdit->clearFilters();
    emit sigFilterChanged(m_activeFilters, ui->lineEdit->text());
    ui->lineEdit->setFocus(Qt::FocusReason::NoFocusReason);
}

/**
 * \brief Sets up filters based on the current widget
 */
void FilterWidget::setupFilters()
{
    switch (m_activeWidget) {
    case MainWidgets::Movies: m_availableFilters = setupMovieFilters(); break;
    case MainWidgets::TvShows: m_availableFilters = setupTvShowFilters(); break;
    case MainWidgets::Concerts: m_availableFilters = setupConcertFilters(); break;
    case MainWidgets::Music: m_availableFilters = setupMusicFilters(); break;
    case MainWidgets::MovieSets:
    case MainWidgets::Genres:
    case MainWidgets::Certifications:
    case MainWidgets::Downloads:
    case MainWidgets::Duplicates:
        // Filtering not possible.
        break;
    }
}

/**
 * \brief Sets up movie filters
 */
QVector<Filter*> FilterWidget::setupMovieFilters()
{
    // Load available genres/directors/etc.

    QStringList years;
    QStringList genres;
    QStringList certifications;
    QStringList studios;
    QStringList countries;
    QStringList tags;
    QStringList directors;
    QStringList videocodecs;
    // TODO: QVector<MovieSet>
    QStringList sets;

    const auto copyNotEmptyUnique = [](const QStringList& from, QStringList& to) {
        for (const QString& str : from) {
            if (!str.isEmpty() && !to.contains(str)) {
                to.append(str);
            }
        }
    };

    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
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
        const auto certStr = movie->certification().toString();
        if (movie->certification().isValid() && !certifications.contains(certStr)) {
            certifications.append(certStr);
        }
        if (!movie->set().name.isEmpty() && !sets.contains(movie->set().name)) {
            sets.append(movie->set().name);
        }
    }

    const auto sortByLocaleCompare = [](QStringList& list) {
        std::sort(list.begin(), list.end(), LocaleStringCompare());
    };
    sortByLocaleCompare(certifications);
    sortByLocaleCompare(genres);
    sortByLocaleCompare(years);
    sortByLocaleCompare(studios);
    sortByLocaleCompare(countries);
    sortByLocaleCompare(tags);
    sortByLocaleCompare(directors);
    sortByLocaleCompare(videocodecs);
    sortByLocaleCompare(sets);

    // Set new filters

    const auto setNewFilters = [](const QStringList& filtersToApply, QString filterTypeName, MovieFilters infoType) {
        QVector<Filter*> newFilters;
        for (const QString& filterName : filtersToApply) {
            newFilters << new Filter(QStringLiteral("%1 \"%2\"").arg(filterTypeName, filterName),
                filterName,
                QStringList() << filterTypeName << filterName,
                infoType,
                true);
        }
        return newFilters;
    };

    // clang-format off
    QVector<Filter *> movieGenreFilters         = setNewFilters(genres,         tr("Genre"),         MovieFilters::Genres);
    QVector<Filter *> movieStudioFilters        = setNewFilters(studios,        tr("Studio"),        MovieFilters::Studio);
    QVector<Filter *> movieCountryFilters       = setNewFilters(countries,      tr("Country"),       MovieFilters::Country);
    QVector<Filter *> movieSetsFilters          = setNewFilters(sets,           tr("Set"),           MovieFilters::Set);
    QVector<Filter *> movieTagsFilters          = setNewFilters(tags,           tr("Tag"),           MovieFilters::Tags);
    QVector<Filter *> movieDirectorFilters      = setNewFilters(directors,      tr("Director"),      MovieFilters::Director);
    QVector<Filter *> movieVideoCodecFilters    = setNewFilters(videocodecs,    tr("Video codec"),   MovieFilters::VideoCodec);
    QVector<Filter *> movieCertificationFilters = setNewFilters(certifications, tr("Certification"), MovieFilters::Certification);
    // clang-format on

    QVector<Filter*> movieYearFilters;
    for (const QString& year : years) {
        movieYearFilters << new Filter(
            tr("Released %1").arg(year), year, QStringList() << tr("Year") << year, MovieFilters::Released, true);
    }

    QVector<Filter*> movieLabelFilters;
    QMapIterator<ColorLabel, QString> it(helper::labels());
    while (it.hasNext()) {
        it.next();
        movieLabelFilters << new Filter(tr("Label \"%1\"").arg(it.value()),
            it.value(),
            QStringList() << tr("Label") << it.value(),
            MovieFilters::Label,
            true,
            it.key());
    }

    return QVector<Filter*>() << m_availableMovieFilters   //
                              << movieGenreFilters         //
                              << movieStudioFilters        //
                              << movieCountryFilters       //
                              << movieYearFilters          //
                              << movieCertificationFilters //
                              << movieSetsFilters          //
                              << movieTagsFilters          //
                              << movieDirectorFilters      //
                              << movieVideoCodecFilters    //
                              << movieLabelFilters;
}

QVector<Filter*> FilterWidget::setupTvShowFilters()
{
    return m_availableTvShowFilters;
}

QVector<Filter*> FilterWidget::setupConcertFilters()
{
    return m_availableConcertFilters;
}

QVector<Filter*> FilterWidget::setupMusicFilters()
{
    return m_availableMusicFilters;
}

/**
 * \brief Initially sets up filters
 */
void FilterWidget::initAvailableFilters()
{
    // clang-format off
    m_availableMovieFilters << new Filter(tr("Title"),                "",               QStringList(),                  MovieFilters::Title,  true);
    m_availableMovieFilters << new Filter(tr("Original Title"),       "",               QStringList(),                  MovieFilters::OriginalTitle, true);
    m_availableMovieFilters << new Filter(tr("Filename"),             "",               QStringList(),                  MovieFilters::Path,   true);
    m_availableMovieFilters << new Filter(tr("IMDb ID"),              "",               QStringList(),                  MovieFilters::ImdbId, true);
    m_availableMovieFilters << new Filter(tr("Movie has no IMDb ID"), tr("No IMDb ID"), {tr("IMDb"), tr("No IMDb ID")}, MovieFilters::ImdbId, false);
    m_availableMovieFilters << new Filter(tr("TMDb ID"),              "",               QStringList(),                  MovieFilters::TmdbId, true);
    m_availableMovieFilters << new Filter(tr("Movie has no TMDb ID"), tr("No TMDb ID"), {tr("TMDb"), tr("No TMDb ID")}, MovieFilters::TmdbId, false);

    // Information
    m_availableMovieFilters << new Filter(tr("Movie has no Studio"),        tr("No Studio"),        {tr("Studio"),        tr("No Studio")},        MovieFilters::Studio,        false);
    m_availableMovieFilters << new Filter(tr("Movie has no Country"),       tr("No Country"),       {tr("Country"),       tr("No Country")},       MovieFilters::Country,       false);
    m_availableMovieFilters << new Filter(tr("Movie has no Director"),      tr("No Director"),      {tr("Director"),      tr("No Director")},      MovieFilters::Director,      false);
    m_availableMovieFilters << new Filter(tr("Movie has no Genre"),         tr("No Genre"),         {tr("Genre"),         tr("No Genre")},         MovieFilters::Genres,        false);
    m_availableMovieFilters << new Filter(tr("Movie has no Tags"),          tr("No Tags"),          {tr("Tags"),          tr("No Tags")},          MovieFilters::Tags,          false);
    m_availableMovieFilters << new Filter(tr("Movie has no Certification"), tr("No Certification"), {tr("Certification"), tr("No Certification")}, MovieFilters::Certification, false);

    // Watched/Rating
    m_availableMovieFilters << new Filter(tr("Movie is Watched"),    tr("Watched"),   {tr("Watched"), tr("Seen")},                                MovieFilters::Watched, true);
    m_availableMovieFilters << new Filter(tr("Movie is Unwatched"),  tr("Unwatched"), {tr("Watched"), tr("Seen"), tr("Unwatched"), tr("Unseen")}, MovieFilters::Watched, false);
    m_availableMovieFilters << new Filter(tr("Movie has Rating"),    tr("Rating"),    {tr("Rating")},                                             MovieFilters::Rating,  true);
    m_availableMovieFilters << new Filter(tr("Movie has no Rating"), tr("No Rating"), {tr("Rating"), tr("No Rating")},                            MovieFilters::Rating,  false);

    // Actors
    m_availableMovieFilters << new Filter(tr("Movie has Actors"),    tr("Actors"),    {"Actors"},                      MovieFilters::Actors, true);
    m_availableMovieFilters << new Filter(tr("Movie has no Actors"), tr("No Actors"), {tr("Actors"), tr("No Actors")}, MovieFilters::Actors, false);

    // Streamdetails
    m_availableMovieFilters << new Filter(tr("Stream Details loaded"),            tr("Stream Details"),    {"Stream Details"},                              MovieFilters::StreamDetails, true);
    m_availableMovieFilters << new Filter(tr("Stream Details not loaded"),        tr("No Stream Details"), {tr("Stream Details"), tr("No Stream Details")}, MovieFilters::StreamDetails, false);
    m_availableMovieFilters << new Filter(tr("No information about video codec"), tr("No video codec"),    {tr("Video codec"), tr("No video codec")},       MovieFilters::VideoCodec,    false);

    // Images
    m_availableMovieFilters << new Filter(tr("Movie has Poster"),           tr("Poster"),           {tr("Poster")},                                                     MovieFilters::Poster,       true);
    m_availableMovieFilters << new Filter(tr("Movie has no Poster"),        tr("No Poster"),        {tr("Poster"), tr("No Poster")},                                    MovieFilters::Poster,       false);
    m_availableMovieFilters << new Filter(tr("Movie has Extra Fanarts"),    tr("Extra Fanarts"),    {tr("Extra Fanarts")},                                              MovieFilters::ExtraFanarts, true);
    m_availableMovieFilters << new Filter(tr("Movie has no Extra Fanarts"), tr("No Extra Fanarts"), {tr("Extra Fanarts"), tr("No Extra Fanarts")},                      MovieFilters::ExtraFanarts, false);
    m_availableMovieFilters << new Filter(tr("Movie has Backdrop"),         tr("Backdrop"),         {tr("Backdrop"), tr("Fanart")},                                     MovieFilters::Backdrop,     true);
    m_availableMovieFilters << new Filter(tr("Movie has no Backdrop"),      tr("No Backdrop"),      {tr("Backdrop"), tr("Fanart"), tr("No Backdrop"), tr("No Fanart")}, MovieFilters::Backdrop,     false);
    m_availableMovieFilters << new Filter(tr("Movie has Logo"),             tr("Logo"),             {tr("Logo")},                                                       MovieFilters::Logo,         true);
    m_availableMovieFilters << new Filter(tr("Movie has no Logo"),          tr("No Logo"),          {tr("Logo"), tr("No Logo")},                                        MovieFilters::Logo,         false);
    m_availableMovieFilters << new Filter(tr("Movie has Clear Art"),        tr("Clear Art"),        {tr("Clear Art")},                                                  MovieFilters::ClearArt,     true);
    m_availableMovieFilters << new Filter(tr("Movie has no Clear Art"),     tr("No Clear Art"),     {tr("Clear Art"), tr("No Clear Art")},                              MovieFilters::ClearArt,     false);
    m_availableMovieFilters << new Filter(tr("Movie has Banner"),           tr("Banner"),           {tr("Banner")},                                                     MovieFilters::Banner,       true);
    m_availableMovieFilters << new Filter(tr("Movie has no Banner"),        tr("No Banner"),        {tr("Banner"), tr("No Banner")},                                    MovieFilters::Banner,       false);
    m_availableMovieFilters << new Filter(tr("Movie has Thumb"),            tr("Thumb"),            {"Thumb"},                                                          MovieFilters::Thumb,        true);
    m_availableMovieFilters << new Filter(tr("Movie has no Thumb"),         tr("No Thumb"),         {tr("Thumb"), tr("No Thumb")},                                      MovieFilters::Thumb,        false);
    m_availableMovieFilters << new Filter(tr("Movie has CD Art"),           tr("CD Art"),           {tr("CD Art")},                                                     MovieFilters::CdArt,        true);
    m_availableMovieFilters << new Filter(tr("Movie has no CD Art"),        tr("No CD Art"),        {tr("CD Art"), tr("No CD Art")},                                    MovieFilters::CdArt,        false);

    // Trailer
    m_availableMovieFilters << new Filter(tr("Movie has Trailer"),          tr("Trailer"),          {tr("Trailer")},                                                                MovieFilters::Trailer,     true);
    m_availableMovieFilters << new Filter(tr("Movie has no Trailer"),       tr("No Trailer"),       {tr("Trailer"), tr("No Trailer")},                                              MovieFilters::Trailer,     false);
    m_availableMovieFilters << new Filter(tr("Movie has local Trailer"),    tr("Local Trailer"),    {tr("Trailer"), tr("Local Trailer")},                                           MovieFilters::LocalTrailer, true);
    m_availableMovieFilters << new Filter(tr("Movie has no local Trailer"), tr("No local Trailer"), {tr("Trailer"), tr("Local Trailer"), tr("No Trailer"), tr("No local Trailer")}, MovieFilters::LocalTrailer, false);

    // Quality
    m_availableMovieFilters << new Filter(tr("Resolution 720p"),  "720p",   {tr("Resolution"), tr("720p")},  MovieFilters::Quality, true);
    m_availableMovieFilters << new Filter(tr("Resolution 1080p"), "1080p",  {tr("Resolution"), tr("1080p")}, MovieFilters::Quality, true);
    m_availableMovieFilters << new Filter(tr("Resolution 2160p"), "2160p",  {tr("Resolution"), tr("2160p")}, MovieFilters::Quality, true);
    m_availableMovieFilters << new Filter(tr("Resolution SD"),    "SD",     {tr("Resolution"), tr("SD")},    MovieFilters::Quality, true);
    m_availableMovieFilters << new Filter(tr("Format DVD"),       "DVD",    {tr("Format"), tr("DVD")},       MovieFilters::Quality, true);
    m_availableMovieFilters << new Filter(tr("BluRay Format"),    "BluRay", {tr("Format"), tr("BluRay")},    MovieFilters::Quality, true);

    // Audiochannels
    m_availableMovieFilters << new Filter(tr("Channels 2.0"),         "2.0",          {tr("Audio"), tr("Channels"), "2.0"},                        MovieFilters::AudioChannels, true);
    m_availableMovieFilters << new Filter(tr("Channels 5.1"),         "5.1",          {tr("Audio"), tr("Channels"), "5.1"},                        MovieFilters::AudioChannels, true);
    m_availableMovieFilters << new Filter(tr("Channels 7.1"),         "7.1",          {tr("Audio"), tr("Channels"), "2.0"},                        MovieFilters::AudioChannels, true);
    m_availableMovieFilters << new Filter(tr("Audio Quality HD"),     "HD Audio",     {tr("Audio"), tr("HD Audio"), "True HD", "DTS HD", "Dolby"}, MovieFilters::AudioQuality,  true);
    m_availableMovieFilters << new Filter(tr("Audio Quality Normal"), "Normal Audio", {tr("Audio"), tr("Normal Audio"), "AAC", "DTS", "AC3", "Dolby"},    MovieFilters::AudioQuality,  true);
    m_availableMovieFilters << new Filter(tr("Audio Quality SD"),     "SD Audio",     {tr("Audio"), tr("SD Audio"), "MP3"},                        MovieFilters::AudioQuality,  true);

    // Subtitle
    m_availableMovieFilters << new Filter(tr("Movie has Subtitle"),               tr("Subtitle"),             {tr("Subtitle")},                                                                         MovieFilters::HasSubtitle,         true);
    m_availableMovieFilters << new Filter(tr("Movie has no Subtitle"),            tr("No Subtitle"),          {tr("No Subtitle"), tr("Subtitle")},                                                      MovieFilters::HasSubtitle,         false);
    m_availableMovieFilters << new Filter(tr("Movie has external Subtitle"),      tr("External Subtitle"),    {tr("Subtitle"), tr("External Subtitle")},                                                MovieFilters::HasExternalSubtitle, true);
    m_availableMovieFilters << new Filter(tr("Movie has no external Subtitle"),   tr("No External Subtitle"), {tr("Subtitle"), tr("External Subtitle"), tr("No Subtitle"), tr("No External Subtitle")}, MovieFilters::HasExternalSubtitle, false);
    // clang-format on

    m_availableTvShowFilters << new Filter(tr("Title"), "", QStringList(), TvShowFilters::Title, true);
    m_availableConcertFilters << new Filter(tr("Title"), "", QStringList(), ConcertFilters::Title, true);
    m_availableMusicFilters << new Filter(tr("Title"), "", QStringList(), MusicFilters::Title, true);
}

/**
 * \brief Store filters for widget
 * \param widget Active widget
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
 * \brief Load filters, stored before
 * \param widget Active widget
 */
void FilterWidget::loadFilters(MainWidgets widget)
{
    if (!m_storedFilters.contains(widget)) {
        return;
    }

    m_activeFilters = m_storedFilters[widget];
    for (Filter* filter : m_activeFilters) {
        ui->lineEdit->addFilter(filter);
    }
}

void FilterWidget::setupFilterListUi()
{
    m_list = new QListWidget();
    m_list->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_list->setAttribute(Qt::WA_ShowWithoutActivating, true);
    m_list->setAttribute(Qt::WA_MacShowFocusRect, false);

    QPalette palette = m_list->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Highlight));
    palette.setColor(QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
    m_list->setPalette(palette);
    m_list->setStyleSheet(QStringLiteral("background-color: #ffffff; border: 1px solid #f0f0f0; border-radius: 5px;"));
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    const qreal pixelRatio = helper::devicePixelRatio(m_list);
    if (pixelRatio >= 0.95 && pixelRatio <= 1.05) {
        // Pixel ratio is 1
        auto* effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(16);
        effect->setOffset(0);
        effect->setColor(QColor(0, 0, 0, 100));
        m_list->setGraphicsEffect(effect);
    }
}
