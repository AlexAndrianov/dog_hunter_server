#include "TCPServer.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    dh::TCPServer server;
    if (!server.listen(QHostAddress::Any, 8080)) {
        std::cout  << "Server could not start!";
        return 1;
    }

    std::cout << "Server started on port 8080";
    return a.exec();
}
