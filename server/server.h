#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QObject>
#include <QList>

// Stub of ChatClient for reference purposes
class ChatClient;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    static void updateClientList();
    static void broadcastMessage(QString);
    static int serverPort;
    static QList<ChatClient*> clients;
    static double Version;
    static QString fileLocation;

private slots:
    void clientDisconnected(ChatClient*);
    void newClient();

private:

    QTcpServer server_socket;
};

#endif // SERVER_H
