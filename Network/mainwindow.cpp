#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <windows.h>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings(QString("../config.ini"), QSettings::IniFormat);
    network_port = settings.value("settings/network_port", "7003").toInt();
    bit_error_rate = settings.value("settings/bit_error_rate", 10).toInt();

    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::AnyIPv4, network_port);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));

    tx_socket = new QUdpSocket(this);
    connect(this, SIGNAL(valueChanged(QString)), ui->textBrowser, SLOT(append(QString)));

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AppendToLog(QString s)
{
    emit valueChanged(s);
}

void MainWindow::readDatagrams()
{
    packet p;

    int randomValue = qrand() % ((100 + 1) - 0) + 0;


    if (socket->hasPendingDatagrams())
    {

        if (randomValue >= bit_error_rate)
        {
            socket->readDatagram((char*)&p, sizeof(p));
            ProcessPacket(p);
        }
        else
        {
            socket->readDatagram((char*)&p, sizeof(p));
            AppendToLog("Dropping packet: ");
            PrintPacketInfo(p);

        }
    }
}

void MainWindow::ProcessPacket(packet p)
{
    PrintPacketInfo(p);
    WriteUDP(p);
}

void MainWindow::PrintPacketInfo(packet p)
{
    QString pType;

    switch (p.PacketType)
    {
        case CONTROL_PACKET:
            pType = "Control";
            break;
        case DATA_PACKET:
            pType = "Data";
            break;
        default:
            pType = "Unknown";
            break;
    }

    QString packetInfo = QString("Packet Type: %1\n"
                                 "Sequence: %2\n"
                                 "WindowSize: %3\n"
                                 "AckNum: %4\n"
                                 "Destination addr: %5\n"
                                 "Destination port: %6\n")
            .arg(pType,
                 QString::number(p.SeqNum),
                 QString::number(p.WindowSize),
                 QString::number(p.AckNum),
                 p.dest_addr,
                 QString::number(p.dest_port));

    AppendToLog(packetInfo);
   // AppendToLog(p.data);
}


void MainWindow::WriteUDP(packet p)
{
    // Redirect packet to the destination address and port in the received packet structure
    Sleep(DELAY);
    tx_socket->writeDatagram( (char*)&p, sizeof(p), QHostAddress(p.dest_addr), p.dest_port);
}
