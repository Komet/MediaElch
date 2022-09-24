#pragma once

#include "data/Actor.h"

#include <QString>
#include <QVector>
#include <memory>
#include <vector>

class MovieCrew
{
public:
    QString writer() const;
    QString director() const;
    Actors& actors();
    const Actors& actors() const;

    void setWriter(QString writer);
    void setDirector(QString director);

    void setActors(QVector<Actor> actors);
    void addActor(Actor actor);
    void removeActor(Actor* actor);

private:
    QString m_writer;
    QString m_director;

    Actors m_actors;
};
