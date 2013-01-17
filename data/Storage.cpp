#include "Storage.h"

Storage::Storage(QObject *parent, Movie *movie) :
    QObject(parent)
{
    m_movie = QPointer<Movie>(movie);
}

Storage::Storage(QObject *parent, Concert *concert) :
    QObject(parent)
{
    m_concert = QPointer<Concert>(concert);
}

Storage::Storage(QObject *parent, QList<ScraperSearchResult> results) :
    QObject(parent)
{
    m_results = results;
}

Movie *Storage::movie()
{
    if (m_movie)
        return m_movie;
    return 0;
}

Concert *Storage::concert()
{
    if (m_concert)
        return m_concert;
    return 0;
}

QVariant Storage::toVariant(QObject *parent, Movie *movie)
{
    Storage *storage = new Storage(parent, movie);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, Concert *concert)
{
    Storage *storage = new Storage(parent, concert);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<ScraperSearchResult> results)
{
    Storage *storage = new Storage(parent, results);
    QVariant var;
    var.setValue(storage);
    return var;
}

QList<ScraperSearchResult> Storage::results()
{
    return m_results;
}
