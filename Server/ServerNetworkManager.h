#ifndef SERVERNETWORKMANAGER_H
#define SERVERNETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include "../Common/protocol.h"

class ServerNetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit ServerNetworkManager(QObject *parent = nullptr);
    bool startServer(quint16 port = 8888);
    void stopServer();

signals:
    void messageReceived(const NetworkMessage &message, QTcpSocket *client);
    void clientConnected(QTcpSocket *client);
    void clientDisconnected(QTcpSocket *client);

public slots:
    void sendMessage(const NetworkMessage &message, QTcpSocket *client);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onDataReceived();

private:
    QTcpServer *tcpServer;
    QList<QTcpSocket*> clients;
};

#endif // SERVERNETWORKMANAGER_H
