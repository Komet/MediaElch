#pragma once

#include "globals/Globals.h"

#include <QList>
#include <QString>

class MovieCrew
{
public:
    QString writer() const;
    QString director() const;
    const QList<Actor> &actors() const;
    QList<Actor> &actors();
    QList<Actor *> actorsPointer();

    void setWriter(QString writer);
    void setDirector(QString director);
    void setActors(QList<Actor> actors);
    void addActor(Actor actor);
    void removeActor(Actor *actor);

private:
    QString m_writer;
    QString m_director;
    QList<Actor> m_actors;
};
