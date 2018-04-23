#include "drawandchatserver.h"
#include "networkinfo.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaEnum>

#include <QDebug>

DrawAndChatServer::DrawAndChatServer(quint16 port, QObject *parent) :
    QObject(parent),
    _webSocketServer("DrawAndChatServer", QWebSocketServer::NonSecureMode, this)
{
    if (_webSocketServer.listen(QHostAddress::Any, port))
    {
        connect(&_webSocketServer, &QWebSocketServer::newConnection,this, &DrawAndChatServer::onNewConnection);
        connect(&_webSocketServer, &QWebSocketServer::closed, this, &DrawAndChatServer::onClosed);
    }
}

DrawAndChatServer::~DrawAndChatServer()
{
    _webSocketServer.close();
}

QJsonDocument DrawAndChatServer::MakeServerJson(const QString &operation, const QJsonObject &arguments)
{
    return QJsonDocument
    {
        QJsonObject
        {
            { "name", NetworkInfo::ServerName },
            { "version",  NetworkInfo::ServerVersion },
            { "operation", operation },
            { "arguments", arguments }
        }
    };
}

void DrawAndChatServer::DebugOutput(QWebSocket *user, const QString &operation)
{
    qDebug()
            << "[" << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss") << "]"
            << user << ":" << operation;
}

DrawAndChatServer::Error DrawAndChatServer::clearIndexInfo(QWebSocket *user)
{
    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend() && !clientInfo->roomName.isNull())
    {
        auto foundRoom = _roomInfoMap.find(clientInfo->roomName);
        if(foundRoom != _roomInfoMap.cend() && !clientInfo->userName.isNull())
        {
            auto foundUser = foundRoom->users.find(clientInfo->userName);
            if(foundUser != foundRoom->users.cend())
            {
                foundRoom->users.erase(foundUser);
                return Error::NoError;
            }
            return Error::NoThisUser;
        }
        return Error::RoomNotFound;
    }
    return Error::ClientNotFound;
}

void DrawAndChatServer::userLoginRoom(QWebSocket *user, const QString &inUserName, const QString &inRoomName, const QString &roomPassword)
{
    if(inUserName.isNull() || inRoomName.isNull())
    {
        userLoginRoomResponse(user, Error::InvaildArgument);
    }

    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend())
    {
        if(!clientInfo->roomName.isNull())
        {
            auto foundRoom = _roomInfoMap.find(clientInfo->roomName);
            if(foundRoom != _roomInfoMap.cend() && !clientInfo->userName.isNull())
            {
                auto foundUser = foundRoom->users.find(clientInfo->userName);
                if(foundUser != foundRoom->users.cend())
                {
                    foundRoom->users.erase(foundUser);
                }
            }
        }

        auto foundNewRoom = _roomInfoMap.find(inRoomName);
        if(foundNewRoom != _roomInfoMap.cend())
        {
            auto foundUser = foundNewRoom->users.find(inUserName);
            if(foundUser == foundNewRoom->users.cend())
            {
                clientInfo->roomName = inRoomName;
                clientInfo->userName = inUserName;

                foundNewRoom->users.insert(inUserName, user);

                otherLoginRoom(user, inUserName);

                userLoginRoomResponse(user, Error::NoError);
            }
            else userLoginRoomResponse(user, Error::UserExisting);
        }
        else userLoginRoomResponse(user, Error::RoomNotFound);
    }
    else userLoginRoomResponse(user, Error::ClientNotFound);
}

void DrawAndChatServer::userCreateRoom(QWebSocket *user, const QString &inUserName, const QString &inRoomName, const QString &roomPassword)
{
    if(inUserName.isNull() || inRoomName.isNull())
    {
        userCreateRoomResponse(user, Error::InvaildArgument);
    }

    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend())
    {
        if(!clientInfo->roomName.isNull())
        {
            auto foundRoom = _roomInfoMap.find(clientInfo->roomName);
            if(foundRoom != _roomInfoMap.cend() && !clientInfo->userName.isNull())
            {
                auto foundUser = foundRoom->users.find(clientInfo->userName);
                if(foundUser != foundRoom->users.cend())
                {
                    foundRoom->users.erase(foundUser);
                }
            }
        }

        auto foundNewRoom = _roomInfoMap.find(inRoomName);
        if(foundNewRoom == _roomInfoMap.cend())
        {
            clientInfo->roomName = inRoomName;
            clientInfo->userName = inUserName;

            _roomInfoMap.insert(inRoomName, RoomInfo(RoomInfo::UsersType{ {inUserName, user} }));

            userCreateRoomResponse(user, Error::NoError);
        }
        else userCreateRoomResponse(user, Error::RoomExisting);
    }
    else userCreateRoomResponse(user, Error::ClientNotFound);
}

void DrawAndChatServer::userPushPaint(QWebSocket *user, int state, const QJsonObject &argList)
{

}

void DrawAndChatServer::userRemovePaint(QWebSocket *user, int id)
{

}

void DrawAndChatServer::userSendMessage(QWebSocket *user, const QString &message)
{
    if(message.isNull())
    {
        userSendMessageResponse(user, Error::InvaildArgument);
    }

    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend() && !clientInfo->roomName.isNull() && !clientInfo->userName.isNull())
    {
        otherSendMessage(user, clientInfo->userName, message);

        userSendMessageResponse(user, Error::NoError);
    }
    else userSendMessageResponse(user, Error::ClientNotFound);
}

void DrawAndChatServer::userLogoutRoom(QWebSocket *user)
{
    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend())
    {
        if(!clientInfo->roomName.isNull())
        {
            auto foundRoom = _roomInfoMap.find(clientInfo->roomName);
            if(foundRoom != _roomInfoMap.cend() && !clientInfo->userName.isNull())
            {
                auto foundUser = foundRoom->users.find(clientInfo->userName);
                if(foundUser != foundRoom->users.cend())
                {
                    foundRoom->users.erase(foundUser);

                    otherLogoutRoom(user, clientInfo->userName);
                }
            }
        }

        clientInfo->roomName.clear();
        clientInfo->userName.clear();
    }

}

void DrawAndChatServer::otherLoginRoomResponse(QWebSocket *user)
{

}

void DrawAndChatServer::otherPushPaintResponse(QWebSocket *user)
{

}

void DrawAndChatServer::otherRemovePaintResponse(QWebSocket *user)
{

}

void DrawAndChatServer::otherSendMessageResponse(QWebSocket *user)
{

}

void DrawAndChatServer::broadcastToUserInRoom(QWebSocket *user, const std::function<void (QWebSocket*)> &action)
{
    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend() && !clientInfo->roomName.isNull())
    {
        auto foundRoom = _roomInfoMap.find(clientInfo->roomName);
        if(foundRoom != _roomInfoMap.cend())
        {
            for(QWebSocket* other : foundRoom->users)
            {
                if(other != user)
                {
                    action(other);
                }
            }
        }
    }
}

void DrawAndChatServer::onNewConnection()
{
    QWebSocket *socket = _webSocketServer.nextPendingConnection();

    DebugOutput(socket, "NewConnection");

    connect(socket, &QWebSocket::binaryMessageReceived, this, &DrawAndChatServer::onMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &DrawAndChatServer::onSocketDisconnected);

    _clientInfoMap.insert(socket, ClientInfo());

    newConnection();
}

void DrawAndChatServer::onClosed()
{
    closed();
}

void DrawAndChatServer::onMessageReceived(const QByteArray &message)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());

    QJsonParseError error;
    QJsonDocument json = QJsonDocument::fromJson(message, &error);

    if(error.error == QJsonParseError::NoError)
    {
        if(json["name"] == NetworkInfo::ClientName && json["version"] == NetworkInfo::ClientVersion)
        {
            static QMap<QString, std::function<void(QWebSocket*, const QJsonObject&)>> operationFunctions{
                {"userLoginRoom", [this](QWebSocket* client, const QJsonObject& arg){
                    userLoginRoom(client, arg["userName"].toString(), arg["roomName"].toString(), arg["roomPassword"].toString());
                }},
                {"userCreateRoom", [this](QWebSocket* client, const QJsonObject& arg){
                    userCreateRoom(client, arg["userName"].toString(), arg["roomName"].toString(), arg["roomPassword"].toString());
                }},
                {"userPushPaint", [this](QWebSocket* client, const QJsonObject& arg){
                    userPushPaint(client, arg["paintState"].toInt(), arg["paintArguments"].toObject());
                }},
                {"userRemovePaint", [this](QWebSocket* client, const QJsonObject& arg){
                    userRemovePaint(client, arg["id"].toInt());
                }},
                {"userSendMessage", [this](QWebSocket* client, const QJsonObject& arg){
                    userSendMessage(client, arg["message"].toString());
                }},
                {"userLogoutRoom", [this](QWebSocket* client, const QJsonObject& arg){
                    userLogoutRoom(client);
                }},
                {"otherLoginRoomResponse", [this](QWebSocket* client, const QJsonObject& arg){
                    otherLoginRoomResponse(client);
                }},
                {"otherPushPaintResponse", [this](QWebSocket* client, const QJsonObject& arg){
                    otherPushPaintResponse(client);
                }},
                {"otherRemovePaintResponse", [this](QWebSocket* client, const QJsonObject& arg){
                    otherRemovePaintResponse(client);
                }},
                {"otherSendMessageResponse", [this](QWebSocket* client, const QJsonObject& arg){
                    otherSendMessageResponse(client);
                }}
            };

            auto operation = json["operation"].toString();
            auto found = operationFunctions.find(operation);
            if(found != operationFunctions.cend())
            {
                DebugOutput(client, operation);
                (*found)(client, json["arguments"].toObject());
            }
        }
    }
}

void DrawAndChatServer::onSocketDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());

    clearIndexInfo(client);

    _clientInfoMap.remove(client);

    DebugOutput(client, "Disconnected");

}

void DrawAndChatServer::userLoginRoomResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userLoginRoomResponse",QJsonObject{
                                            {"state", state},
                                            {"error", QMetaEnum::fromType<Error>().valueToKey(state)}
                                        });

    user->sendBinaryMessage(json.toJson());

}

void DrawAndChatServer::userCreateRoomResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userCreateRoomResponse",QJsonObject{
                                            {"state", state},
                                            {"error", QMetaEnum::fromType<Error>().valueToKey(state)}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::userPushPaintResponse(QWebSocket *user, int state, int id)
{
    QJsonDocument json = MakeServerJson("userPushPaintResponse",QJsonObject{
                                            {"state", state},
                                            {"error", QMetaEnum::fromType<Error>().valueToKey(state)},
                                            {"id", id}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::userRemovePaintResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userRemovePaintResponse",QJsonObject{
                                            {"state", state},
                                            {"error", QMetaEnum::fromType<Error>().valueToKey(state)}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::userSendMessageResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userSendMessageResponse",QJsonObject{
                                            {"state", state},
                                            {"error", QMetaEnum::fromType<Error>().valueToKey(state)}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::otherLoginRoom(QWebSocket *user, const QString &inUserName)
{
    QJsonDocument json = MakeServerJson("otherLoginRoom",QJsonObject{
                                            {"userName", inUserName}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](QWebSocket* user){
        user->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::otherPushPaint(QWebSocket *user, int id, int state, const QJsonObject &argList)
{
    QJsonDocument json = MakeServerJson("otherPushPaint",QJsonObject{
                                            {"id", id},
                                            {"paintState", state},
                                            {"paintArguments", argList}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](QWebSocket* user){
        user->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::otherRemovePaint(QWebSocket *user, int id)
{
    QJsonDocument json = MakeServerJson("otherRemovePaint",QJsonObject{
                                            {"id", id}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](QWebSocket* user){
        user->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::otherSendMessage(QWebSocket *user, const QString &inUserName, const QString &message)
{
    QJsonDocument json = MakeServerJson("otherSendMessage",QJsonObject{
                                            {"userName", inUserName},
                                            {"message", message}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](QWebSocket* user){
        user->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::otherLogoutRoom(QWebSocket *user, const QString &inUserName)
{
    QJsonDocument json = MakeServerJson("otherLogoutRoom",QJsonObject{
                                            {"userName", inUserName}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](QWebSocket* user){
        user->sendBinaryMessage(jsonBytes);
    });
}
