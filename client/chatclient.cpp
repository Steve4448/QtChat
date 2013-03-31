#include "chatclient.h"
#include "ui_chatclient.h"

#include <QDebug>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QUrl>
double ChatClient::Version = 1.0;

ChatClient::ChatClient(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ChatClient)
{
	ui->setupUi(this);
	ui->inputMessageArea->installEventFilter(this);

	connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui->actionClear_Chat, SIGNAL(triggered()), ui->messageArea, SLOT(clear()));
	connect(ui->actionClear_Chat, SIGNAL(triggered()), ui->inputMessageArea, SLOT(clear()));
	connect(ui->connectNowButton, SIGNAL(clicked()), this, SLOT(connectToHost()));

	connect(&socket, SIGNAL(connected()), this, SLOT(connected()));
	connect(&socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(readData()));

	connect(&timer, SIGNAL(timeout()), this, SLOT(connectToHost()));
	timer.setInterval(60000); // 1 minute interval
	timer.setParent(this);
	connectToHost();
}

void ChatClient::connectionError(QAbstractSocket::SocketError) {
	qCritical() << "Connection error:" << socket.errorString();
	addText("<font color=\"#FF0000\">Connection error:</font> " + socket.errorString());
	timer.start();
}

void ChatClient::connectToHost() {
	addText("Attempting to connect...");
	socket.connectToHost("localhost", 8612);
}

void ChatClient::connected() {
	ui->inputMessageArea->setEnabled(true);
	ui->connectNowButton->setEnabled(false);
	ui->connectNowButton->setVisible(false);
	doPacket(0x1, "");
	timer.stop();
	addText("<font color=\"#005500\">Connection established.</font>");
}

void ChatClient::disconnected() {
	ui->inputMessageArea->setEnabled(false);
	ui->connectNowButton->setVisible(true);
	ui->connectNowButton->setEnabled(true);
	ui->usersOnlineList->clear();
	timer.start();
	addText("<font color=\"#FF0000\">Lost connection to server</font>, retrying every minute.");
}

bool ChatClient::eventFilter(QObject*, QEvent* ev) {
	if(ev->type() == QEvent::KeyPress){
		QKeyEvent* kevent = ((QKeyEvent*)ev);
		if(kevent->modifiers().testFlag(Qt::ShiftModifier))
			return false;
		if(kevent->key() == Qt::Key_Return || kevent->key() == Qt::Key_Enter) {
			sendMessage();
			return true;
		}
	}
	return false;
}

void ChatClient::sendMessage() {
	QString t = ui->inputMessageArea->toPlainText();
	if(t.trimmed() != "") {
		doPacket(0x2, t);
		ui->inputMessageArea->clear();
	}
}

void ChatClient::readData() {
	QByteArray packet = socket.readAll();
	QDataStream reader(&packet, QIODevice::ReadOnly);
	while(!reader.atEnd()) {
		quint8 opcode;
		reader >> opcode;
		//qDebug() << "Got Packet" << QString::number(opcode, 16);
		switch(opcode) {
			case 0x1: // username packet
			{
				int numClients;
				reader >> numClients;
				ui->usersOnlineList->clear();
				for(int i = 0; i < numClients; i++) {
					QString usern;
					reader >> usern;
					ui->usersOnlineList->addItem(usern);
				}
			}
			break;
			case 0x2: //Message packet.
			{
				QString nmess;
				reader >> nmess;
				qDebug() << "Message:" << nmess;
				addText(nmess);
			}
			break;
			default:
			{
				qCritical() << "Malformed packet?" << QString::number(opcode, 16);
				abort();
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
		case 0x1:
		{
			stream << ChatClient::Version;
		}
		break;
		case 0x2:
		{
			stream << stuff;
		}
		break;
	}
	socket.write(packet);
}

void ChatClient::addText(QString text) {
	ui->messageArea->append(text);
}

ChatClient::~ChatClient() {
	delete ui;
}
