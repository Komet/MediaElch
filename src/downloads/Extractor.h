#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVector>

class Extractor : public QObject
{
    Q_OBJECT
public:
    explicit Extractor(QObject* parent = nullptr);
    ~Extractor() override;

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
    QVector<QProcess*> m_processes;
};
