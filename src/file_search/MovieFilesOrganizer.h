#pragma once

#include "media/Path.h"

#include <QString>
#include <QThread>

/**
 * \brief The MovieFilesOrganizer class
 */
class MovieFilesOrganizer : public QThread
{
    Q_OBJECT
public:
    explicit MovieFilesOrganizer(QObject* parent = nullptr);
    ~MovieFilesOrganizer() override = default;
    void canceled(QString msg);
    void moveToDirs(mediaelch::DirectoryPath dir);
};
