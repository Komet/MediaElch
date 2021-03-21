#include "ui/small_widgets/RatingsWidget.h"
#include "ui_RatingsWidget.h"

#include "data/RatingModel.h"
#include "ui/small_widgets/RatingSourceComboDelegate.h"
#include "ui/small_widgets/SpinBoxDelegate.h"

#include <QDebug>

RatingsWidget::RatingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::RatingsWidget)
{
    ui->setupUi(this);

    m_ratingModel = new RatingModel(this);
    ui->ratings->setModel(m_ratingModel);
    ui->ratings->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ratings->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->ratings->setItemDelegateForColumn(RatingModel::SourceColumn, new RatingSourceComboDelegate(this));
    ui->ratings->setItemDelegateForColumn(RatingModel::RatingColumn, new DoubleSpinBoxDelegate(0.1, this));
    ui->ratings->setItemDelegateForColumn(RatingModel::VoteCountColumn, new SpinBoxDelegate(this));
    ui->ratings->setColumnHidden(RatingModel::MaxRatingColumn, true);
    ui->ratings->setColumnHidden(RatingModel::MinRatingColumn, true);

    connect(ui->btnAddRating, &QAbstractButton::clicked, this, &RatingsWidget::addRating);
    connect(ui->btnRemoveRating, &QAbstractButton::clicked, this, &RatingsWidget::removeRating);
    connect(m_ratingModel, &RatingModel::dataChanged, this, [this]() { emit ratingsChanged(); });
}

RatingsWidget::~RatingsWidget()
{
    delete ui;
}

void RatingsWidget::clear()
{
    bool blocked = ui->ratings->blockSignals(true);
    m_ratingModel->setRatings(nullptr);
    ui->ratings->blockSignals(blocked);
}

void RatingsWidget::setRatings(Ratings* ratings)
{
    m_ratingModel->setRatings(ratings);
}

void RatingsWidget::addRating()
{
    Rating r;
    r.source = "default";
    r.rating = 0.0;
    r.voteCount = 0;
    r.minRating = 0.0;
    r.maxRating = 10.0;

    m_ratingModel->addRating(r);

    ui->ratings->scrollToBottom();
    ui->ratings->selectRow(m_ratingModel->rowCount() - 1);

    emit ratingsChanged();
}

void RatingsWidget::removeRating()
{
    QModelIndex index = ui->ratings->selectionModel()->currentIndex();
    if (!ui->ratings->selectionModel()->hasSelection() || !index.isValid()) {
        qInfo() << "[RatingsWidget] Cannot remove rating because none is selected!";
        return;
    }

    m_ratingModel->removeRows(index.row(), 1);

    emit ratingsChanged();
}
