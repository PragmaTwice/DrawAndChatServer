#include "clientinfo.h"

ClientInfo::ClientInfo(const QString &inRoomName, const QString &inUserName):
    roomName(inRoomName),
    userName(inUserName)
{
}

ClientInfo::~ClientInfo()
{
}
