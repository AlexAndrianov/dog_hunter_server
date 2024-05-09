#pragma once
#include "Database.h"
#include "RequestHandler.h"

#include <QtCore>
#include <QtNetwork>

namespace dh {

class TCPServer : public QTcpServer {
    Q_OBJECT
public:

    explicit TCPServer(QObject *parent = nullptr)
        : QTcpServer(parent) {}

    ~TCPServer()
    { }

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

                QJsonParseError error;
                auto jsonDoc = QJsonDocument::fromJson(requestData, &error);

                if (error.error != QJsonParseError::NoError) {
                    qDebug() << "Error parsing JSON:" << error.errorString();

                    socket->write(QString("Wrong data format").toUtf8());
                    socket->flush();
                    return;
                }

                auto requestHandler = RequestHandler::create(jsonDoc);
                RequestHandlerPtr responce;

                if(auto loginRequest =  qSharedPointerCast<LoginRequest>(requestHandler))
                {
                    auto &db = Database::getInstance();

                    Database::QueryResult res;
                    auto dogOwner = db.getDogOwner(loginRequest->_login.toStdString(), loginRequest->_password.toStdString(), res);

                    auto loginResponce = QSharedPointer<LoginResponce>::create();

                    if(res == Database::QueryResult::Ok)
                    {
                        loginResponce->_status = LoginResponce::ResponceStatus::Valid;
                        loginResponce->_dogOwner = dogOwner;
                    }
                    else if(res == Database::QueryResult::WrongLogin)
                    {
                        loginResponce->_status = LoginResponce::ResponceStatus::InvalidLogin;
                    }
                    else if(res == Database::QueryResult::WrongPassword)
                    {
                        loginResponce->_status = LoginResponce::ResponceStatus::InvalidPassword;
                    }
                    else
                    {
                        socket->write(QString("Database error! Finishing server.").toUtf8());
                        socket->flush();
                        return;
                    }

                    responce = loginResponce;
                }

                if(!responce)
                {
                    socket->write(QString("Undefined behaviour").toUtf8());
                    socket->flush();
                    return;
                }

                QJsonDocument jsonDocOut(responce->serialize());
                QByteArray requestDataOut = jsonDocOut.toJson(QJsonDocument::Indented);
                // Echo the received message back to the client

                socket->write(requestDataOut);
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
