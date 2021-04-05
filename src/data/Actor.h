#pragma once

#include <QDebug>
#include <QMetaType>
#include <QString>
#include <QVector>

struct Actor
{
    QString name;
    QString role;
    QString thumb;
    QByteArray image;
    QString id;
    int order = 0; // used by Kodi NFO
    bool imageHasChanged = false;
};


class Actors : public QObject
{
    Q_OBJECT
public:
    Actors(QObject* parent = nullptr) : QObject(parent) {}
    ~Actors() override;

    void setActors(QVector<Actor> actors);
    void addActor(Actor actor);
    /// \brief Removes the given actor. Actors are compared by pointer and not by value.
    void removeActor(Actor* actor);

    /// \brief Clears all images from all actors.
    void clearImages();

    void removeAll();

    const QVector<Actor*>& actors();
    QVector<const Actor*> actors() const;

private:
    QVector<Actor*> m_actors;
};


Q_DECLARE_METATYPE(Actor*)
Q_DECLARE_METATYPE(QString*)
Q_DECLARE_METATYPE(QVector<int>)

QDebug operator<<(QDebug dbg, const Actor& actor);
