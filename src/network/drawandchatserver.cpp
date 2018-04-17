#include "drawandchatserver.h"
#include "networkinfo.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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

Error DrawAndChatServer::clearIndexInfo(QWebSocket *user)
{
    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend() && !clientInfo->first.isNull())
    {
        auto foundRoom = _roomUserIndexMap.find(clientInfo->first);
        if(foundRoom != _roomUserIndexMap.cend() && !clientInfo->second.isNull())
        {
            auto foundUser = foundRoom->find(clientInfo->second);
            if(foundUser != foundRoom->cend())
            {
                foundRoom->erase(foundUser);
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
        if(!clientInfo->first.isNull())
        {
            auto foundRoom = _roomUserIndexMap.find(clientInfo->first);
            if(foundRoom != _roomUserIndexMap.cend() && !clientInfo->second.isNull())
            {
                auto foundUser = foundRoom->find(clientInfo->second);
                if(foundUser != foundRoom->cend())
                {
                    foundRoom->erase(foundUser);
                }
            }
        }

        auto foundNewRoom = _roomUserIndexMap.find(inRoomName);
        if(foundNewRoom != _roomUserIndexMap.cend())
        {
            clientInfo->first = inRoomName;
            clientInfo->second = inUserName;

            foundNewRoom->insert(inUserName, user);

            otherLoginRoom(user, inUserName);

            userLoginRoomResponse(user, Error::NoError);
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
        if(!clientInfo->first.isNull())
        {
            auto foundRoom = _roomUserIndexMap.find(clientInfo->first);
            if(foundRoom != _roomUserIndexMap.cend() && !clientInfo->second.isNull())
            {
                auto foundUser = foundRoom->find(clientInfo->second);
                if(foundUser != foundRoom->cend())
                {
                    foundRoom->erase(foundUser);
                }
            }
        }

        auto foundNewRoom = _roomUserIndexMap.find(inRoomName);
        if(foundNewRoom == _roomUserIndexMap.cend())
        {
            clientInfo->first = inRoomName;
            clientInfo->second = inUserName;

            _roomUserIndexMap.insert(inRoomName, QMap<QString, QWebSocket *>{ {inUserName, user} });

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
    if(clientInfo != _clientInfoMap.cend() && !clientInfo->first.isNull() && !clientInfo->second.isNull())
    {
        otherSendMessage(user, clientInfo->second, message);

        userSendMessageResponse(user, Error::NoError);
    }
    else userSendMessageResponse(user, Error::ClientNotFound);
}

void DrawAndChatServer::userLogoutRoom(QWebSocket *user)
{
    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend())
    {
        if(!clientInfo->first.isNull())
        {
            auto foundRoom = _roomUserIndexMap.find(clientInfo->first);
            if(foundRoom != _roomUserIndexMap.cend() && !clientInfo->second.isNull())
            {
                auto foundUser = foundRoom->find(clientInfo->second);
                if(foundUser != foundRoom->cend())
                {
                    foundRoom->erase(foundUser);

                    otherLogoutRoom(user, clientInfo->second);
                }
            }
        }

        clientInfo->first.clear();
        clientInfo->second.clear();
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

void DrawAndChatServer::BroadcastToUserInRoom(QWebSocket *user, const std::function<void (const UserIndexPair &)> &action)
{
    auto clientInfo = _clientInfoMap.find(user);
    if(clientInfo != _clientInfoMap.cend() && !clientInfo->first.isNull())
    {
        auto foundRoom = _roomUserIndexMap.find(clientInfo->first);
        if(foundRoom != _roomUserIndexMap.cend())
        {
            for(const UserIndexPair& userIndex : *foundRoom)
            {
                if(userIndex.second != user)
                {
                    action(userIndex);
                }
            }
        }
    }
}

void DrawAndChatServer::onNewConnection()
{
    QWebSocket *socket = _webSocketServer->nextPendingConnection();

    connect(socket, &QWebSocket::binaryMessageReceived, this, &DrawAndChatServer::onMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &DrawAndChatServer::onSocketDisconnected);

    _clientInfoMap.insert(socket, ClientInfoPair{});

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
            static QMap<QString, std::function<void(const QJsonObject&)>> operationFunctions{
                {"userLoginRoom", [this,client](const QJsonObject& arg){
                    userLoginRoom(client, arg["userName"].toString(), arg["roomName"].toString(), arg["roomPassword"].toString());
                }},
                {"userCreateRoom", [this,client](const QJsonObject& arg){
                    userCreateRoom(client, arg["userName"].toString(), arg["roomName"].toString(), arg["roomPassword"].toString());
                }},
                {"userPushPaint", [this,client](const QJsonObject& arg){
                    userPushPaint(client, arg["paintState"].toInt(), arg["paintArguments"]);
                }},
                {"userRemovePaint", [this,client](const QJsonObject& arg){
                    userRemovePaint(client, arg["id"].toInt());
                }},
                {"userSendMessage", [this,client](const QJsonObject& arg){
                    userSendMessage(client, arg["message"].toString());
                }},
                {"userLogoutRoom", [this,client](const QJsonObject& arg){
                    userLogoutRoom(client);
                }},
                {"otherLoginRoomResponse", [this,client](const QJsonObject& arg){
                    otherLoginRoomResponse(client);
                }},
                {"otherPushPaintResponse", [this,client](const QJsonObject& arg){
                    otherPushPaintResponse(client);
                }},
                {"otherRemovePaintResponse", [this,client](const QJsonObject& arg){
                    otherRemovePaintResponse(client);
                }},
                {"otherSendMessageResponse", [this,client](const QJsonObject& arg){
                    otherSendMessageResponse(client);
                }},
                {"requestErrorMessage", [this,client](const QJsonObject& arg){
                    requestErrorMessage(client, arg["errorState"].toInt());
                }}
            };

            auto found = operationFunctions.find(json["operation"].toString());
            if(found != operationFunctions.cend())
            {
                (*found)(json["arguments"].toObject());
            }
        }
    }
}

void DrawAndChatServer::onSocketDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());

    clearIndexInfo(client);

    _clientInfoMap.remove(client);
}

void DrawAndChatServer::userLoginRoomResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userLoginRoomResponse",QJsonObject{
                                            {"state", state}
                                        });

    user->sendBinaryMessage(json.toJson());

}

void DrawAndChatServer::userCreateRoomResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userCreateRoomResponse",QJsonObject{
                                            {"state", state}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::userPushPaintResponse(QWebSocket *user, int state, int id)
{
    QJsonDocument json = MakeServerJson("userPushPaintResponse",QJsonObject{
                                            {"state", state},
                                            {"id", id}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::userRemovePaintResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userRemovePaintResponse",QJsonObject{
                                            {"state", state}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::userSendMessageResponse(QWebSocket *user, int state)
{
    QJsonDocument json = MakeServerJson("userSendMessageResponse",QJsonObject{
                                            {"state", state}
                                        });

    user->sendBinaryMessage(json.toJson());
}

void DrawAndChatServer::otherLoginRoom(QWebSocket *user, const QString &inUserName)
{
    QJsonDocument json = MakeServerJson("otherLoginRoom",QJsonObject{
                                            {"userName", inUserName}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](const UserIndexPair& pair){
        pair.second->sendBinaryMessage(jsonBytes);
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

    broadcastToUserInRoom(user, [&jsonBytes](const UserIndexPair& pair){
        pair.second->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::otherRemovePaint(QWebSocket *user, int id)
{
    QJsonDocument json = MakeServerJson("otherRemovePaint",QJsonObject{
                                            {"id", id}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](const UserIndexPair& pair){
        pair.second->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::otherSendMessage(QWebSocket *user, const QString &inUserName, const QString &message)
{
    QJsonDocument json = MakeServerJson("otherSendMessage",QJsonObject{
                                            {"userName", inUserName},
                                            {"message", message}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](const UserIndexPair& pair){
        pair.second->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::otherLogoutRoom(QWebSocket *user, const QString &inUserName)
{
    QJsonDocument json = MakeServerJson("otherLogoutRoom",QJsonObject{
                                            {"userName", inUserName}
                                        });

    QByteArray jsonBytes = json.toJson();

    broadcastToUserInRoom(user, [&jsonBytes](const UserIndexPair& pair){
        pair.second->sendBinaryMessage(jsonBytes);
    });
}

void DrawAndChatServer::requestErrorMessageResponse(QWebSocket *user, const QString &errorString)
{
    QJsonDocument json = MakeServerJson("requestErrorMessageResponse",QJsonObject{
                                            {"errorString", errorString}
                                        });

    user->sendBinaryMessage(json.toJson());
}
