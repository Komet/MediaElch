#ifndef XBMCXML_H
#define XBMCXML_H

#include <QObject>

#include "data/MediaCenterInterface.h"
#include "data/Movie.h"

class XbmcXml : public QObject, public MediaCenterInterface
{
    Q_OBJECT
public:
    explicit XbmcXml(QObject *parent = 0);
    ~XbmcXml();

    bool saveData(Movie *movie);
    bool loadData(Movie *movie);
    void loadImages(Movie *movie);
};

#endif // XBMCXML_H
