#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QTcpSocket>
#include <QObject>

class ChatClient : public QObject
{
    Q_OBJECT
public:
    explicit ChatClient(QObject *parent, QTcpSocket* sock);
    inline void send(QByteArray* data){socket->write(*data);socket->flush();}
    virtual ~ChatClient();
    QString username;
    QString ipAddress;
    void doPacket(quint8, QString);

signals:
    void disconnected(ChatClient*);

private slots:
    void initData();
    void newData();

private:
    static uint id;
    QTcpSocket* socket;
};

#endif // CHATCLIENT_H
