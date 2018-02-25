#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class Extractor : public QObject
{
    Q_OBJECT
public:
    explicit Extractor(QObject *parent = nullptr);
    ~Extractor();

public slots:
    void extract(QString baseName, QStringList files, QString password);
    void stopExtraction(QString baseName);

signals:
    void sigProgress(QString, int);
    void sigFinished(QString, bool);
    void sigError(QString, QString);

private slots:
    void onReadyRead();
    void onReadyReadError();
    void onFinished(int exitCode, QProcess::ExitStatus status);

private:
    QList<QProcess*> m_processes;
};

#endif // EXTRACTOR_H
