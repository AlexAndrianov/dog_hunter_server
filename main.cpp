#include "TCPServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    dh::TCPServer server;
    if (!server.listen(QHostAddress::LocalHost, 8080)) {
        qDebug() << "Server could not start!";
        return 1;
    }

    if(!server.isListening())
    {
        qDebug() << "Server didn't start!";
        return 1;
    }

    qDebug() << "Server started on port 8080";
    auto res = a.exec();

    qDebug() << "Server finished on port 8080";
    return res;
}
