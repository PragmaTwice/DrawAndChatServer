#include <QCoreApplication>
#include <QLoggingCategory>
#include "network/drawandchatserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    DrawAndChatServer server(2333, &a);

    return a.exec();
}
