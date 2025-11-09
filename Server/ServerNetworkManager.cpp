#include "servernetworkmanager.h"
#include <QDebug>

ServerNetworkManager::ServerNetworkManager(QObject *parent)
    : QObject(parent), tcpServer(new QTcpServer(this))
{
    connect(tcpServer, &QTcpServer::newConnection, this, &ServerNetworkManager::onNewConnection);
}

bool ServerNetworkManager::startServer(quint16 port)
{
    if (!tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "服务器启动失败:" << tcpServer->errorString();
        return false;
    }
    qDebug() << "服务器启动成功，端口:" << port;
    return true;
}

void ServerNetworkManager::onNewConnection()
{
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
    clients.append(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead, this, &ServerNetworkManager::onDataReceived);
    connect(clientSocket, &QTcpSocket::disconnected, this, &ServerNetworkManager::onClientDisconnected);

    qDebug() << "客户端连接:" << clientSocket->peerAddress().toString();
    emit clientConnected(clientSocket);
}

void ServerNetworkManager::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        clients.removeAll(clientSocket);
        clientSocket->deleteLater();
        qDebug() << "客户端断开连接";
        emit clientDisconnected(clientSocket);
    }
}

void ServerNetworkManager::onDataReceived()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    NetworkMessage message = NetworkMessage::fromJson(data);

    qDebug() << "收到客户端消息，类型:" << message.type;
    emit messageReceived(message, clientSocket);
}

void ServerNetworkManager::sendMessage(const NetworkMessage &message, QTcpSocket *client)
{
    if (client && client->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = message.toJson();
        client->write(data);
        qDebug() << "向客户端发送消息，类型:" << message.type;
    }
}
