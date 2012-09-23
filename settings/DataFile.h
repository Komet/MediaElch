#ifndef DATAFILE_H
#define DATAFILE_H

#include <QString>

/**
 * @brief The DataFile class
 */
class DataFile
{
public:
    DataFile(int id, QString name, QString fileName, int pos, bool enabled);

    QString name();
    QString fileName();
    int pos() const;
    int id();
    bool enabled();
    QString saveFileName(QString fileName);

    void setEnabled(bool enabled);
    void setPos(int pos);

    static bool lessThan(const DataFile *a, const DataFile *b);

private:
    QString m_name;
    QString m_fileName;
    int m_pos;
    int m_id;
    bool m_enabled;
};

#endif // DATAFILE_H
