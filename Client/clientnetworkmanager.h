#ifndef CLIENTNETWORKMANAGER_H
#define CLIENTNETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include "../Common/protocol.h"

class ClientNetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit ClientNetworkManager(QObject *parent = nullptr);
    void connectToServer(const QString &host = "127.0.0.1", quint16 port = 8888);
    void disconnectFromServer();
    void sendMessage(const NetworkMessage &message);
    bool isConnected() const;

signals:
    void messageReceived(const NetworkMessage &message);
    void connected();
    void disconnected();
    void connectionError(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *tcpSocket;
};

#endif // CLIENTNETWORKMANAGER_H
