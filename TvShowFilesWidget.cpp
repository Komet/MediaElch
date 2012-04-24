#include "TvShowFilesWidget.h"
#include "ui_TvShowFilesWidget.h"

#include <QDebug>
#include "Manager.h"
#include "data/TvShowModelItem.h"

TvShowFilesWidget::TvShowFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowFilesWidget)
{
    ui->setupUi(this);

#ifdef Q_WS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif
#ifdef Q_WS_WIN
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif

    m_tvShowDelegate = new TvShowDelegate(this);
    m_tvShowProxyModel = new TvShowProxyModel(this);
    m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
    m_tvShowProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_tvShowProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_tvShowProxyModel);
    ui->files->setItemDelegate(m_tvShowDelegate);
    ui->files->sortByColumn(0);

    connect(ui->files, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(finished()), this, SLOT(searchFinished()));
    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemActivated(QModelIndex,QModelIndex)));
}

TvShowFilesWidget::~TvShowFilesWidget()
{
    delete ui;
}
void TvShowFilesWidget::setFilter(QString filter)
{
    m_tvShowProxyModel->setFilterWildcard("*" + filter + "*");
}

void TvShowFilesWidget::enableRefresh()
{
    emit setRefreshButtonEnabled(true, WidgetTvShows);
}

void TvShowFilesWidget::disableRefresh()
{
    emit setRefreshButtonEnabled(false, WidgetTvShows);
}

void TvShowFilesWidget::startSearch()
{
    emit setRefreshButtonEnabled(false, WidgetTvShows);
    Manager::instance()->tvShowFileSearcher()->start();
}

void TvShowFilesWidget::searchFinished()
{
    emit setRefreshButtonEnabled(true, WidgetTvShows);
}

void TvShowFilesWidget::renewModel()
{
    m_tvShowProxyModel->setSourceModel(0);
    m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
}

void TvShowFilesWidget::onItemClicked(QModelIndex index)
{
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(index);
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        bool wasExpanded = ui->files->isExpanded(index);
        ui->files->collapseAll();
        if (!wasExpanded)
            ui->files->expand(index);
    }
}

void TvShowFilesWidget::onItemActivated(QModelIndex index, QModelIndex previous)
{
    Q_UNUSED(previous);

    if (!index.isValid()) {
        emit sigNothingSelected();
        return;
    }

    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(index);
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        emit sigTvShowSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShow());
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeEpisode) {
        emit sigEpisodeSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShowEpisode());
    }
}
