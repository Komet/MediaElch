#ifndef MOVIEFILESORGANIZER_H
#define MOVIEFILESORGANIZER_H

#include <QDir>
#include <QThread>

#include "globals/Globals.h"
#include "data/Movie.h"

/**
 * @brief The MovieFilesOrganizer class
 */
class MovieFilesOrganizer : public QThread
{
    Q_OBJECT
public:
    explicit MovieFilesOrganizer(QObject *parent = nullptr);
    ~MovieFilesOrganizer() override = default;
    void canceled(QString msg);
    void moveToDirs(QString dir);
};

#endif // MOVIEFILESORGANIZER_H
