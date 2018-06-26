#include "MovieDuplicates.h"
#include "ui_MovieDuplicates.h"

#include "../globals/Helper.h"
#include "../globals/Manager.h"
#include "../notifications/NotificationBox.h"
#include "MovieDuplicateItem.h"
#include <QDebug>

MovieDuplicates::MovieDuplicates(QWidget *parent) : QWidget(parent), ui(new Ui::MovieDuplicates)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont font = ui->movies->font();
    font.setPointSize(font.pointSize());
    ui->movies->setFont(font);
#endif

    if (!Settings::instance()->movieDuplicatesSplitterState().isNull())
        ui->splitter->restoreState(Settings::instance()->movieDuplicatesSplitterState());
    else
        ui->splitter->setSizes(QList<int>() << 200 << 600);

    m_movieProxyModel = new MovieProxyModel(this);
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
    m_movieProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_movieProxyModel->setFilterDuplicates(true);
    ui->movies->setModel(m_movieProxyModel);
    for (int i = 1, n = ui->movies->model()->columnCount(); i < n; ++i) {
        ui->movies->setColumnWidth(i, 24);
        ui->movies->setColumnHidden(i, true);
    }
    ui->movies->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#ifdef Q_OS_WIN
    ui->movies->setIconSize(QSize(12, 12));
#else
    ui->movies->setIconSize(QSize(16, 16));
#endif

    ui->duplicates->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    Helper::instance()->applyStyle(ui->movieDuplicatesWidget);

    connect(ui->btnDetect, &QPushButton::clicked, this, &MovieDuplicates::detectDuplicates);
    connect(
        ui->movies->selectionModel(), &QItemSelectionModel::currentChanged, this, &MovieDuplicates::onItemActivated);
}

MovieDuplicates::~MovieDuplicates()
{
    Settings::instance()->setMovieDuplicatesSplitterState(ui->splitter->saveState());
    delete ui;
}

void MovieDuplicates::detectDuplicates()
{
    qDebug() << "Detecting duplicates";
    ui->duplicates->clear();
    ui->duplicates->setRowCount(0);
    m_duplicateMovies.clear();

    int counter = 0;
    int movieCount = Manager::instance()->movieModel()->movies().count();
    NotificationBox::instance()->showProgressBar(
        tr("Detecting duplicate movies..."), Constants::MovieDuplicatesProgressMessageId);
    NotificationBox::instance()->progressBarProgress(0, movieCount, Constants::MovieDuplicatesProgressMessageId);
    qApp->processEvents();

    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        counter++;
        NotificationBox::instance()->progressBarProgress(
            counter, movieCount, Constants::MovieDuplicatesProgressMessageId);
        movie->setHasDuplicates(false);

        QList<Movie *> dups;
        dups << movie;
        foreach (Movie *subMovie, Manager::instance()->movieModel()->movies()) {
            if (movie == subMovie)
                continue;
            if (subMovie->isDuplicate(movie))
                dups.append(subMovie);
        }
        if (dups.count() > 1) {
            m_duplicateMovies.insert(movie, dups);
            movie->setHasDuplicates(true);
        }
    }

    NotificationBox::instance()->hideProgressBar(Constants::MovieDuplicatesProgressMessageId);
}

void MovieDuplicates::onItemActivated(QModelIndex index, QModelIndex previous)
{
    Q_UNUSED(previous)

    if (!index.isValid())
        return;

    int row = index.model()->data(index, Qt::UserRole).toInt();
    Movie *movie = Manager::instance()->movieModel()->movie(row);

    if (!m_duplicateMovies.contains(movie))
        return;

    ui->duplicates->clear();
    ui->duplicates->setRowCount(0);

    foreach (Movie *dup, m_duplicateMovies[movie]) {
        auto item = new MovieDuplicateItem(ui->duplicates);
        item->setMovie(dup, dup == movie);
        item->setDuplicateProperties(movie->duplicateProperties(dup));

        int row = ui->duplicates->rowCount();
        ui->duplicates->insertRow(row);
        ui->duplicates->setCellWidget(row, 0, item);
    }
}
