#ifndef DRAWANDCHATSERVER_H
#define DRAWANDCHATSERVER_H

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

#include <functional>

#include "clientinfo.h"
#include "roominfo.h"

class DrawAndChatServer : public QObject
{
    Q_OBJECT
public:

    explicit DrawAndChatServer(quint16 port, QObject *parent = nullptr);
    ~DrawAndChatServer();

    enum Error
    {
        NoError = 0,
        InvaildArgument,
        NoThisUser,
        RoomNotFound,
        RoomExisting,
        ClientNotFound,
        UserExisting
    };
    Q_ENUM(Error)

private:

    QWebSocketServer _webSocketServer;
    QMap<QWebSocket *, ClientInfo> _clientInfoMap;

    QMap<QString, RoomInfo> _roomInfoMap;

    static QJsonDocument MakeServerJson(const QString &operation, const QJsonObject &arguments);
    static void DebugOutput(QWebSocket *user, const QString& operation);

    void broadcastToUserInRoom(QWebSocket *user, const std::function<void(QWebSocket*)>& action);
    Error clearIndexInfo(QWebSocket *user);

private slots:

    void userLoginRoom(QWebSocket *user, const QString& inUserName, const QString& inRoomName, const QString& roomPassword);
    void userCreateRoom(QWebSocket *user, const QString& inUserName, const QString& inRoomName, const QString& roomPassword);
    void userPushPaint(QWebSocket *user, int state, const QJsonObject &argList);
    void userRemovePaint(QWebSocket *user, int id);
    void userSendMessage(QWebSocket *user, const QString& message);
    void userLogoutRoom(QWebSocket *user);

    void otherLoginRoomResponse(QWebSocket *user);
    void otherPushPaintResponse(QWebSocket *user);
    void otherRemovePaintResponse(QWebSocket *user);
    void otherSendMessageResponse(QWebSocket *user);


private slots:
    void onNewConnection();
    void onClosed();

    void onMessageReceived(const QByteArray &message);
    void onSocketDisconnected();

signals:

    void newConnection();
    void closed();

public slots:

    void userLoginRoomResponse(QWebSocket *user, int state);
    void userCreateRoomResponse(QWebSocket *user, int state);
    void userPushPaintResponse(QWebSocket *user, int state, int id);
    void userRemovePaintResponse(QWebSocket *user, int state);
    void userSendMessageResponse(QWebSocket *user, int state);

    void otherLoginRoom(QWebSocket *user, const QString& inUserName);
    void otherPushPaint(QWebSocket *user, int id, int state, const QJsonObject &argList);
    void otherRemovePaint(QWebSocket *user, int id);
    void otherSendMessage(QWebSocket *user, const QString& inUserName, const QString& message);
    void otherLogoutRoom(QWebSocket *user, const QString& inUserName);

};

#endif // DRAWANDCHATSERVER_H
