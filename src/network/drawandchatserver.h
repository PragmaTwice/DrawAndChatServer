#ifndef DRAWANDCHATSERVER_H
#define DRAWANDCHATSERVER_H

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

class DrawAndChatServer : public QObject
{
    Q_OBJECT
public:

    explicit DrawAndChatServer(quint16 port, QObject *parent = nullptr);
    ~DrawAndChatServer();

private:

    QWebSocketServer _webSocketServer;
    QMap<QWebSocket *, QPair<QString, QString>> _clients;

    QMap<QString, QMap<QString, QWebSocket *>> _roomUserInfo;

signals:

private slots:
    void onNewConnection();
    void onClosed();

    void onMessageReceived(const QByteArray &message);
    void onSocketDisconnected();

public slots:
    void newConnection();
    void closed();

};

#endif // DRAWANDCHATSERVER_H
