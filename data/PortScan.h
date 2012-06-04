#ifndef PORTSCAN_H
#define PORTSCAN_H

#include <QTcpSocket>
#include <QThread>
#include <QUrl>

class PortScan : public QThread
{
    Q_OBJECT
public:
    explicit PortScan(QObject *parent = 0);
    void setUrl(QUrl url, int timeout);
signals:
    void result(bool);
protected:
    void run();
private:
    int m_timeout;
    QUrl m_url;
    QTcpSocket *m_socket;
};

#endif // PORTSCAN_H
