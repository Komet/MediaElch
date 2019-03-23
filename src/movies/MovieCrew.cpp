#include "MovieCrew.h"


QString MovieCrew::writer() const
{
    return m_writer;
}

QString MovieCrew::director() const
{
    return m_director;
}

const QVector<Actor>& MovieCrew::actors() const
{
    return m_actors;
}

QVector<Actor>& MovieCrew::actors()
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
    m_actors = actors;
}

void MovieCrew::addActor(Actor actor)
{
    m_actors.append(actor);
}

void MovieCrew::removeActor(Actor* actor)
{
    for (int i = 0, n = m_actors.size(); i < n; ++i) {
        if (&m_actors[i] == actor) {
            m_actors.removeAt(i);
            break;
        }
    }
}
