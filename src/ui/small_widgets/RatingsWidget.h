#pragma once
#include <QWidget>

#include "data/Rating.h"

namespace Ui {
class RatingsWidget;
}

class RatingModel;

class RatingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RatingsWidget(QWidget* parent = nullptr);
    ~RatingsWidget();

    void clear();
    void setRatings(Ratings* ratings);

signals:
    void ratingsChanged();

private slots:
    void addRating();
    void removeRating();

private:
    Ui::RatingsWidget* ui;

    RatingModel* m_ratingModel = nullptr;
};
