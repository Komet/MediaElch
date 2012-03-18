#ifndef XBMCXML_H
#define XBMCXML_H

#include <QObject>
#include <QXmlStreamWriter>

#include "data/MediaCenterInterface.h"
#include "data/Movie.h"

class XbmcXml : public MediaCenterInterface
{
    Q_OBJECT
public:
    explicit XbmcXml(QObject *parent = 0);
    ~XbmcXml();

    bool saveData(Movie *movie);
    bool loadData(Movie *movie);
    void loadImages(Movie *movie);
    void exportDatabase(QList<Movie *> movies, QString exportPath, QString pathSearch, QString pathReplace);
signals:
    void sigExportStarted();
    void sigExportProgress(int, int);
    void sigExportDone();
    void sigExportRaiseError(QString);
private:
    void writeXml(QXmlStreamWriter &xml, Movie *movie, bool writePath = false, QString pathSearch = "", QString pathReplace = "");
};

#endif // XBMCXML_H
