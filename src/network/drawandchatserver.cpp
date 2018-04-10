#include "drawandchatserver.h"

DrawAndChatServer::DrawAndChatServer(quint16 port, QObject *parent) :
    QObject(parent),
    _webSocketServer("DrawAndChatServer", QWebSocketServer::NonSecureMode, this)
{
    if (_webSocketServer->listen(QHostAddress::Any, port))
    {
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &DrawAndChatServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &DrawAndChatServer::onClosed);
    }
}

DrawAndChatServer::~DrawAndChatServer()
{
    _webSocketServer->close();
}

void DrawAndChatServer::onNewConnection()
{
    QWebSocket *socket = _webSocketServer->nextPendingConnection();

    connect(socket, &QWebSocket::binaryMessageReceived, this, &DrawAndChatServer::onMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &DrawAndChatServer::onSocketDisconnected);

    newConnection();
}

void DrawAndChatServer::onClosed()
{
    closed();
}

void DrawAndChatServer::onMessageReceived(const QByteArray &message)
{

}

void DrawAndChatServer::onSocketDisconnected()
{

}
