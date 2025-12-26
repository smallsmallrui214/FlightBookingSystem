#include "clientnetworkmanager.h"
#include <QDebug>

ClientNetworkManager::ClientNetworkManager(QObject *parent)
    : QObject(parent), tcpSocket(new QTcpSocket(this))
{
    connect(tcpSocket, &QTcpSocket::connected, this, &ClientNetworkManager::onConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &ClientNetworkManager::onDisconnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &ClientNetworkManager::onDataReceived);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &ClientNetworkManager::onError);
}

void ClientNetworkManager::connectToServer(const QString &host, quint16 port)
{
    qDebug() << "尝试连接到服务器:" << host << ":" << port;
    tcpSocket->connectToHost(host, port);
}

void ClientNetworkManager::disconnectFromServer()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    } else {
        tcpSocket->abort();
    }
}

bool ClientNetworkManager::isConnected() const
{
    return tcpSocket->state() == QAbstractSocket::ConnectedState;
}

void ClientNetworkManager::onConnected()
{
    qDebug() << "成功连接到服务器";
    emit connected();
}

void ClientNetworkManager::onDisconnected()
{
    qDebug() << "与服务器断开连接";
    emit disconnected();
}

void ClientNetworkManager::onDataReceived()
{
    QByteArray data = tcpSocket->readAll();
    NetworkMessage message = NetworkMessage::fromJson(data);

    qDebug() << "收到服务器消息，类型:" << message.type;
    emit messageReceived(message);
}

void ClientNetworkManager::sendMessage(const NetworkMessage &message)
{
    if (isConnected()) {
        QByteArray data = message.toJson();

        // 添加详细调试
        qDebug() << "=== 发送消息调试 ===";
        qDebug() << "消息类型:" << message.type;
        qDebug() << "消息数据键:" << message.data.keys();
        qDebug() << "JSON内容:" << data;
        qDebug() << "JSON是否为空:" << data.isEmpty();
        qDebug() << "JSON长度:" << data.length();

        tcpSocket->write(data);
        qDebug() << "向服务器发送消息，类型:" << message.type;
    } else {
        qDebug() << "未连接到服务器，无法发送消息";
    }
}

void ClientNetworkManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QString errorStr = tcpSocket->errorString();
    qDebug() << "连接错误:" << errorStr;
    emit connectionError(errorStr);
}
