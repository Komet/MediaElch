#include "MovieCrew.h"


QString MovieCrew::writer() const
{
    return m_writer;
}

QString MovieCrew::director() const
{
    return m_director;
}

Actors& MovieCrew::actors()
{
    return m_actors;
}

const Actors& MovieCrew::actors() const
{
    return m_actors;
}

void MovieCrew::setWriter(QString writer)
{
    m_writer = writer;
}

void MovieCrew::setDirector(QString director)
{
    m_director = director;
}

void MovieCrew::setActors(QVector<Actor> actors)
{
    m_actors.setActors(actors);
}

void MovieCrew::addActor(Actor actor)
{
    m_actors.addActor(actor);
}

void MovieCrew::removeActor(Actor* actor)
{
    m_actors.removeActor(actor);
}
