#include "listenthread.h"
#include "globals.h"

ListenThread::ListenThread(QObject *parent)
{
    window = (MainWindow*)parent;

}

void ListenThread::readrxDatagrams()
{
    MainWindow::packet packet;

    while (rx_socket->hasPendingDatagrams())
    {
        rx_socket->readDatagram((char*)&packet, sizeof(packet));
        window->ProcessPacket(packet);
    }
}

void ListenThread::run()
{
    rx_socket = new QUdpSocket();
    rx_socket->bind(QHostAddress::AnyIPv4, window->receiver_port);
    connect(rx_socket, SIGNAL(readyRead()), this, SLOT(readrxDatagrams()), Qt::DirectConnection);

    exec();
}
