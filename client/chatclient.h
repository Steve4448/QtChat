#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QTcpSocket>
#include <QMainWindow>
#include <QTimer>

namespace Ui {
    class ChatClient;
}

class ChatClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatClient(QWidget *parent = 0);
    ~ChatClient();
    static double Version;

     bool eventFilter(QObject *, QEvent *);

private slots:
    void readData();
    void doPacket(quint8, QString);
    void addText(QString);
    void sendMessage();

    void connectToHost();
    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError);
    void closeEvent(QCloseEvent *event);

private:
    Ui::ChatClient *ui;
    QTcpSocket socket;
    QString username;
    QTimer timer;
};

#endif // CHATCLIENT_H
