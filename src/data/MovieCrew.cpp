#include "MovieCrew.h"


QString MovieCrew::writer() const
{
    return m_writer;
}

QString MovieCrew::director() const
{
    return m_director;
}

const QList<Actor> &MovieCrew::actors() const
{
    return m_actors;
}

QList<Actor> &MovieCrew::actors()
{
    return m_actors;
}

QList<Actor *> MovieCrew::actorsPointer()
{
    QList<Actor *> actors;
    for (int i = 0, n = m_actors.size(); i < n; i++) {
        actors.append(&(m_actors[i]));
    }
    return actors;
}

void MovieCrew::setWriter(QString writer)
{
    m_writer = writer;
}


void MovieCrew::setDirector(QString director)
{
    m_director = director;
}

void MovieCrew::setActors(QList<Actor> actors)
{
    m_actors = actors;
}

void MovieCrew::addActor(Actor actor)
{
}

void MovieCrew::removeActor(Actor *actor)
{
    for (int i = 0, n = m_actors.size(); i < n; ++i) {
        if (&m_actors[i] == actor) {
            m_actors.removeAt(i);
            break;
        }
    }
}
