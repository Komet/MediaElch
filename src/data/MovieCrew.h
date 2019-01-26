#pragma once

#include "globals/Globals.h"

#include <QString>
#include <QVector>

class MovieCrew
{
public:
    QString writer() const;
    QString director() const;
    const QVector<Actor> &actors() const;
    QVector<Actor> &actors();
    QVector<Actor *> actorsPointer();

    void setWriter(QString writer);
    void setDirector(QString director);
    void setActors(QVector<Actor> actors);
    void addActor(Actor actor);
    void removeActor(Actor *actor);

private:
    QString m_writer;
    QString m_director;
    QVector<Actor> m_actors;
};
