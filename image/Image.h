#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>

class Image : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool deletion READ deletion WRITE setDeletion NOTIFY deletionChanged)
    Q_PROPERTY(QByteArray rawData READ rawData WRITE setRawData NOTIFY rawDataChanged)
    Q_PROPERTY(int imageId READ imageId CONSTANT)

public:
    explicit Image(QObject *parent = 0);

    QString fileName() const;
    void setFileName(const QString &fileName);

    bool deletion() const;
    void setDeletion(bool deletion);

    QByteArray rawData() const;
    void setRawData(const QByteArray &rawData);

    int imageId();

    void load();

    void resetIdCounter();

signals:
    void fileNameChanged();
    void deletionChanged();
    void rawDataChanged();

private:
    QString m_fileName;
    bool m_deletion;
    QByteArray m_rawData;
    int m_imageId;
    static int m_idCounter;
};

#endif // IMAGE_H
