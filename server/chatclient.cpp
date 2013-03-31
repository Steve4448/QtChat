#include "chatclient.h"
#include "server.h"

#include <QHostAddress>
#include <QStringList>
#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QDebug>


uint ChatClient::id = 1;

ChatClient::ChatClient(QObject *parent, QTcpSocket* sock) :
    QObject(parent)
{
    socket = sock;
    ipAddress = socket->peerAddress().toString();

    QDir saveLocation(Server::fileLocation);
    QString name = ipAddress;
    QFile file(saveLocation.filePath(name + ".dat"));
    if(file.exists() && file.open(QIODevice::ReadOnly)){
        username = QString::fromUtf8(file.readLine());
        file.close();
    } else username = QString("User %1").arg(id++);

    qDebug() << username << "connected from" << ipAddress;

    connect(socket, SIGNAL(readyRead()), this, SLOT(initData()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

void ChatClient::initData() {
    QByteArray packet = socket->readAll();
    QDataStream reader(&packet, QIODevice::ReadOnly);
    while(!reader.atEnd()) {
        quint8 opcode;
        reader >> opcode;
        switch(opcode) {
            case 0x1:
            {
                double clientVersion;
                reader >> clientVersion;
                if(clientVersion != Server::Version)
                    deleteLater();
                else {
                    // Switches packet handling.
                    connect(socket, SIGNAL(readyRead()), this, SLOT(newData()));
                    disconnect(socket, SIGNAL(readyRead()), this, SLOT(initData()));
                    doPacket(0x1, username);
                    QString mes = QString("<b>%1</b> has connected.").arg(username);
                    Server::broadcastMessage(mes);
                }
            }
            break;
            default:
            {
                deleteLater();
            }
            break;
        }
    }
}

void ChatClient::newData() {
    QByteArray packet = socket->readAll();
    QDataStream reader(&packet, QIODevice::ReadOnly);
    while(!reader.atEnd()) {
        quint8 opcode;
        reader >> opcode;
        //qDebug() << "Got Packet" << QString::number(opcode, 16);
        switch(opcode) {
            case 0x2: // message packet
            {
                QString message;
                reader >> message;
                if(message.startsWith("/") && !message.startsWith("//")){
                    QStringList args = message.split(" ");
                    QString command = args[0];
                    command = command.right(command.length() - 1).toLower();
                    if(command == "name") {
                        if(args.length() < 2) {
                            doPacket(0x2, "Try as: /name NEW NAME");
                            return;
                        }
                        QString newName = "";
                        for(int i = 1; i < args.length(); i++)
                            newName += (i > 1 ? " " : "") + args[i];
                        if(username == newName) {
                            doPacket(0x2, "Why would you try to set your name to a name you already have? Silly...");
                            return;
                        }
                        if(newName.trimmed().length() == 0 || newName.length() >= 12) {
                            doPacket(0x2, "<font color=\"#550000\">Invaild name</font>; The name \"" + newName + "\" is too " + QString(newName.trimmed().length() == 0 ? "short" : "long") + ".");
                            return;
                        }
                        Server::broadcastMessage("<b>" + username + "</b> is now known as <b>" + newName + "</b>.");
                        username = newName;
                        Server::updateClientList();
                    } else doPacket(0x2, "Unknown command: " + command);
                } else {
                    QString word = "";
                    QString output = "";
                    QRegExp splitter("[ ,]");
                    foreach(QChar c, message){
                        if(splitter.exactMatch(c)) {
                            if(word.length() > 0) {
                                QString nword = word;
                                if(nword.indexOf("://") != -1) {
                                    nword = "<a href=\"" + nword + "\">" + nword + "</a>";
                                }
                                output += nword;
                                word = "";
                            }
                            output += c;
                        } else word += c;
                    }
                    if(word.length() > 0) {
                        QString nword = word;
                        if(nword.indexOf("://") != -1) {
                            nword = "<a href=\"" + nword + "\">" + nword + "</a>";
                        }
                        output += nword;
                        word = "";
                    }
                    message = output;

                    message = message.trimmed();
                    if(message.startsWith("//"))
                        message.right(message.length() - 1);
                    message = "<b>" + username + "</b>: " + message;
                    message.replace("\n", "<br />");
                    Server::broadcastMessage(message);
                }
            }
            break;
            default: //Invaild packet?
            {
                deleteLater();
            }
            break;
        }
    }
}

void ChatClient::doPacket(quint8 id, QString stuff) {
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << id;
    switch(id) {
        case 0x1: //Userlist packet
        {
            stream << Server::clients.size();
            //Obtain a user-list to send..
            foreach(ChatClient* client, Server::clients)
                stream << client->username;
        }
        break;
        case 0x2: //Message packet.
        {
            stream << stuff;
        }
        break;

    }
    socket->write(packet);
}

ChatClient::~ChatClient() { // Instance destructer
    Server::broadcastMessage("<b>" + username + "</b> has disconnected.");
    qDebug() << username << "disconnected";
    QDir saveLocation(Server::fileLocation);
    if(!saveLocation.exists())
        qDebug() << "Creating save folder, result: " << (saveLocation.mkpath(".") ? "success." : "failure!" );
    QString name = ipAddress;
    QFile file(saveLocation.filePath(name + ".dat"));
    if(file.open(QIODevice::WriteOnly)) {
        file.write(username.toUtf8());
        file.close();
    }
    emit disconnected(this);
    delete socket;
}
