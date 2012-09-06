#include "TvShowFilesWidget.h"
#include "ui_TvShowFilesWidget.h"

#include <QDebug>
#include "Manager.h"
#include "data/TvShowModelItem.h"

TvShowFilesWidget *TvShowFilesWidget::m_instance;

/**
 * @brief TvShowFilesWidget::TvShowFilesWidget
 * @param parent
 */
TvShowFilesWidget::TvShowFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowFilesWidget)
{
    m_instance = this;
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
    m_tvShowProxyModel = Manager::instance()->tvShowProxyModel();
    m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
    m_tvShowProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_tvShowProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_tvShowProxyModel);
    ui->files->setItemDelegate(m_tvShowDelegate);
    ui->files->sortByColumn(0);
    ui->files->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->files, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemActivated(QModelIndex,QModelIndex)));
}

/**
 * @brief TvShowFilesWidget::~TvShowFilesWidget
 */
TvShowFilesWidget::~TvShowFilesWidget()
{
    delete ui;
}

/**
 * @brief Returns the current instance of the widget
 * @return Current instance of TvShowFilesWidget
 */
TvShowFilesWidget *TvShowFilesWidget::instance()
{
    return m_instance;
}

/**
 * @brief Sets a filter
 * @param filter Filter text
 */
void TvShowFilesWidget::setFilter(QString filter)
{
    m_tvShowProxyModel->setFilterWildcard("*" + filter + "*");
}

/**
 * @brief Renews the model (necessary after searching for tv shows)
 */
void TvShowFilesWidget::renewModel()
{
    m_tvShowProxyModel->setSourceModel(0);
    m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
}

/**
 * @brief Collapses or expands items
 * @param index
 */
void TvShowFilesWidget::onItemClicked(QModelIndex index)
{
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(index);
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        bool wasExpanded = ui->files->isExpanded(index);
        ui->files->collapseAll();
        if (!wasExpanded)
            ui->files->expand(index);
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeSeason) {
        bool wasExpanded = ui->files->isExpanded(index);
        if (wasExpanded)
            ui->files->collapse(index);
        else
            ui->files->expand(index);
    }
}

/**
 * @brief Emits sigTvShowSelected or sigEpisodeSelected
 * @param index
 * @param previous
 */
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
