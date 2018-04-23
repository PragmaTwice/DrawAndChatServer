#include "roominfo.h"

RoomInfo::RoomInfo(const RoomInfo::UsersType &inUsers, const QString &inPassword) :
    users(inUsers),
    password(inPassword)
{
}

RoomInfo::~RoomInfo()
{
}
