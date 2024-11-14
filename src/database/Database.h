#pragma once

#include "data/TmdbId.h"
#include "database/DatabaseId.h"
#include "globals/Globals.h"
#include "media/Path.h"

#include <QSqlDatabase>
#include <QString>
#include <memory>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject* parent = nullptr);
    ~Database() override;

    /// \brief Create a new connection for the calling thread.
    static Database* newConnection(QObject* parent);

    QSqlDatabase db();

    void addImport(QString fileName, QString type, mediaelch::DirectoryPath path);
    bool guessImport(QString fileName, QString& type, QString& path);

    void setLabel(const mediaelch::FileList& fileNames, ColorLabel color);
    ColorLabel getLabel(const mediaelch::FileList& fileNames);

private:
    void setupDatabase();

private:
    mediaelch::DirectoryPath m_dataLocation;
    std::unique_ptr<QSqlDatabase> m_db;
    void updateDbVersion(int version);
};
