#pragma once

#include "media/Path.h"

#include <QObject>

class Image : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool deletion READ deletion WRITE setDeletion NOTIFY deletionChanged)
    Q_PROPERTY(QByteArray rawData READ rawData WRITE setRawData NOTIFY rawDataChanged)
    Q_PROPERTY(int imageId READ imageId CONSTANT)

public:
    explicit Image(QObject* parent = nullptr);

    mediaelch::FilePath filePath() const;
    void setFilePath(const mediaelch::FilePath& filePath);

    bool deletion() const;
    void setDeletion(bool deletion);

    QByteArray rawData() const;
    void setRawData(const QByteArray& rawData);

    int imageId() const;

    void load();

    void resetIdCounter();

signals:
    void fileNameChanged();
    void deletionChanged();
    void rawDataChanged();

private:
    mediaelch::FilePath m_filePath;
    bool m_deletion;
    QByteArray m_rawData;
    int m_imageId;
};
