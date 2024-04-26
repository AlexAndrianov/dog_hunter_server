#pragma once

#include <QtCore>
#include <QtNetwork>

namespace dh {

class TCPServer : public QTcpServer {
    Q_OBJECT
public:

    explicit TCPServer(QObject *parent = nullptr)
        : QTcpServer(parent) {}

protected:

    void incomingConnection(qintptr socketDescriptor) override {
        // New thread for each incoming connection
        QThread* thread = new QThread();
        QTcpSocket* socket = new QTcpSocket();

        socket->setSocketDescriptor(socketDescriptor);
        socket->moveToThread(thread);

        connect(thread, &QThread::started, [=]() {

            // on recieve data
            connect(socket, &QTcpSocket::readyRead, [=]() {
                QByteArray requestData = socket->readAll();
                qDebug() << "Received request:" << requestData;

                // Echo the received message back to the client
                socket->write(requestData);
                socket->flush();
            });

            // on disconnect
            connect(socket, &QTcpSocket::disconnected, [=]() {
                socket->deleteLater();

                thread->quit();
                thread->deleteLater();
            });
        });

        thread->start();
    }
};

}
