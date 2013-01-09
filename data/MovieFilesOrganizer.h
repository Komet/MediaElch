#ifndef MOVIEFILESORGANIZER_H
#define MOVIEFILESORGANIZER_H

#include <QThread>
#include <QDir>

#include "movies/Movie.h"
#include "globals/Globals.h"

/**
 * @brief The MovieFilesOrganizer class
 */
class MovieFilesOrganizer : public QThread
{
    Q_OBJECT
public:
    explicit MovieFilesOrganizer(QObject *parent = 0);
    ~MovieFilesOrganizer();
    void canceled(QString msg);
    void moveToDirs(QString dir);
};

#endif // MOVIEFILESORGANIZER_H

