#ifndef DRAWANDCHATSERVER_H
#define DRAWANDCHATSERVER_H

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

#include <functional>

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
        ClientNotFound
    };
    Q_ENUMS(Error)

private:

    typedef QPair<QString, QString> ClientInfoPair;
    typedef QPair<QString, QWebSocket *> UserIndexPair;

    QWebSocketServer _webSocketServer;
    QMap<QWebSocket *, QPair<QString, QString>> _clientInfoMap;

    QMap<QString, QMap<QString, QWebSocket *>> _roomUserIndexMap;

    static QJsonDocument MakeServerJson(const QString &operation, const QJsonObject &arguments);
    void broadcastToUserInRoom(QWebSocket *user, const std::function<void(const UserIndexPair&)>& action);
    void clearIndexInfo(QWebSocket *user);

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

    void requestErrorMessage(QWebSocket *user, int errorState);

private slots:
    void onNewConnection();
    void onClosed();

    void onMessageReceived(const QByteArray &message);
    void onSocketDisconnected();

public slots:
    void newConnection();
    void closed();

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

    void requestErrorMessageResponse(QWebSocket *user, const QString& errorString);

};

#endif // DRAWANDCHATSERVER_H
