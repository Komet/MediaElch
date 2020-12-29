#pragma once

#include "globals/Globals.h"

#include <QDate>
#include <QDomDocument>
#include <QString>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlReader
{
public:
    explicit MovieXmlReader(Movie& movie);
    void parseNfoDom(QDomDocument domDoc);

private:
    template<class T>
    using MovieStoreMethod = void (Movie::*)(T);

    template<MovieStoreMethod<QString> method>
    void simpleString(const QDomElement& element)
    {
        const QString value = element.text();
        (m_movie.*method)(value);
    }

    template<MovieStoreMethod<QString> method, const char splitChar>
    void stringList(const QDomElement& element)
    {
        QStringList values = element.text().split(splitChar, ElchSplitBehavior::SkipEmptyParts);
        for (const QString& value : asConst(values)) {
            (m_movie.*method)(value.trimmed());
        }
    }

    template<MovieStoreMethod<int> method>
    void simpleInt(const QDomElement& element)
    {
        (m_movie.*method)(element.text().toInt());
    }

    template<MovieStoreMethod<double> method>
    void simpleDouble(const QDomElement& element)
    {
        (m_movie.*method)(element.text().toDouble());
    }

    template<MovieStoreMethod<QDateTime> method>
    void simpleDateTime(const QDomElement& element)
    {
        const QDateTime value = QDateTime::fromString(element.text(), "yyyy-MM-dd HH:mm:ss");
        if (value.isValid()) {
            (m_movie.*method)(value);
        }
    }

    void movieSet(const QDomElement& element);
    void movieActor(const QDomElement& element);
    void movieThumbnail(const QDomElement& element);
    void movieFanart(const QDomElement& element);
    void movieRatingV17(const QDomElement& element);
    void movieRatingV16(const QDomElement& element);
    void movieVoteCountV16(const QDomElement& element);
    void movieResumeTime(const QDomElement& element);

    Movie& m_movie;
};

} // namespace kodi
} // namespace mediaelch
