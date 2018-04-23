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
    QString password;

    RoomInfo(const UsersType& inUsers = UsersType(), const QString& inPassword = QString());
    ~RoomInfo();
};

#endif // ROOMINFO_H
