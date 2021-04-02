#pragma once
#include <QWidget>

#include <QItemSelection>
#include <QString>

namespace Ui {
class ActorsWidget;
}

class ActorModel;
struct Actor;
class Movie;

class ActorsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ActorsWidget(QWidget* parent = nullptr);
    ~ActorsWidget();

public:
    void setMovie(Movie* movie);

signals:
    void actorsChanged();

public slots:
    void clear();

private slots:
    void addActor();
    void removeActor();
    void onActorSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onChangeActorImage();
    void onActorEdited();

private:
    void updateActorImage(Actor* actor);

private:
    Ui::ActorsWidget* ui;
    ActorModel* m_actorModel = nullptr;
    Movie* m_movie = nullptr;
};
