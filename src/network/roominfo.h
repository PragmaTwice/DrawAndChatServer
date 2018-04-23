#ifndef ROOMINFO_H
#define ROOMINFO_H

#include <QMap>
#include <QString>
#include <QWebSocket>

class RoomInfo
{

public:

    typedef QMap<QString, QWebSocket *> UsersType;
    typedef QMap<qint32, QVariantMap> DrawDataType;

    UsersType users;
    DrawDataType drawData;

    RoomInfo(const UsersType& inUsers = UsersType());
    ~RoomInfo();
};

#endif // ROOMINFO_H
