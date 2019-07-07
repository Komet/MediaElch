#pragma once

#include <QDate>
#include <QDomDocument>
#include <QString>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlReader
{
public:
    MovieXmlReader(Movie& movie);
    void parseNfoDom(QDomDocument domDoc);

private:
    template<class T>
    using MovieStoreMethod = void (Movie::*)(T);

    template<MovieStoreMethod<QString> method>
    void simpleString(QDomElement element)
    {
        const QString value = element.text();
        (m_movie.*method)(value);
    }

    template<MovieStoreMethod<QString> method, const char splitChar>
    void stringList(QDomElement element)
    {
        QStringList values = element.text().split(splitChar, QString::SkipEmptyParts);
        for (const QString& value : values) {
            (m_movie.*method)(value.trimmed());
        }
    }

    template<MovieStoreMethod<int> method>
    void simpleInt(QDomElement element)
    {
        (m_movie.*method)(element.text().toInt());
    }

    template<MovieStoreMethod<QDate> method>
    void simpleYear(QDomElement element)
    {
        const QDate value = QDate::fromString(element.text(), "yyyy");
        (m_movie.*method)(value);
    }

    template<MovieStoreMethod<QDate> method>
    void simpleDate(QDomElement element)
    {
        const QDate value = QDate::fromString(element.text(), "yyyy-MM-dd");
        (m_movie.*method)(value);
    }

    void movieSet(QDomElement element);
    void movieActor(QDomElement element);
    void movieThumbnail(QDomElement element);
    void movieFanart(QDomElement element);
    void movieRatingV17(QDomElement element);
    void movieRatingV16(QDomElement element);
    void movieVoteCountV16(QDomElement element);

    Movie& m_movie;
};

} // namespace kodi
} // namespace mediaelch
