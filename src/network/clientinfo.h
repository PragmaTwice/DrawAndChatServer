#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include <QString>

class ClientInfo
{

public:
    QString roomName;
    QString userName;

    ClientInfo(const QString& inRoomName = QString(), const QString& inUserName = QString());
    ~ClientInfo();

};

#endif // CLIENTINFO_H
