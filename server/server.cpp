#include "server.h"

#include <QDebug>

#include "chatclient.h"

int Server::serverPort = 8612;
double Server::Version = 1.0;
QString Server::fileLocation = "saves";
QList<ChatClient*> Server::clients;

Server::Server(QObject *parent) :
    QObject(parent)
{
    if(!server_socket.listen(QHostAddress::Any, serverPort)) {
        qCritical() << "Failed to bind port" << serverPort << ":" << server_socket.errorString();
        abort();
    } else
        qDebug() << "Server now listening on" << serverPort << ".";

    connect(&server_socket, SIGNAL(newConnection()), this, SLOT(newClient()));
}

void Server::broadcastMessage(QString message) {
    foreach(ChatClient* client, clients) {
        client->doPacket((quint8)0x2, message);
    }
}

void Server::updateClientList() {
    foreach(ChatClient* client, clients) {
        client->doPacket((quint8)0x1, (QString)"");
    }
}

void Server::newClient() {
    ChatClient* client = new ChatClient(this, server_socket.nextPendingConnection());
    connect(client, SIGNAL(disconnected(ChatClient*)), this, SLOT(clientDisconnected(ChatClient*)));
    clients << client;
    updateClientList();
}

void Server::clientDisconnected(ChatClient *client) {
    // Remove the disconnected client from the clients list
    clients.removeOne(client);
    updateClientList();
}
