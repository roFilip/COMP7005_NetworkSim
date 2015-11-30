#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <Windows.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listView->setEditTriggers(QListView::NoEditTriggers);
    loadFiles();
    updateFileList();

    tx_socket = new QUdpSocket(this);
    tx_socket->bind(QHostAddress::AnyIPv4, TRANSMIT_PORT);
    connect(tx_socket, SIGNAL(readyRead()), this, SLOT(readtxDatagrams()));

    rx_socket = new QUdpSocket(this);
    rx_socket->bind(QHostAddress::AnyIPv4, RECEIVER_PORT);
    connect(rx_socket, SIGNAL(readyRead()), this, SLOT(readrxDatagrams()));

    connect(this, SIGNAL(valueChanged(QString)), ui->log, SLOT(append(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateFileList()
{
    //Add to the list view
    ui->listView->setModel(new QStringListModel(_fileList));
}

void MainWindow::AppendToLog(QString s)
{
    emit valueChanged(s);
}


void MainWindow::loadFiles()
{
    QDirIterator dirIter("../Files", QDirIterator::Subdirectories);
    QString curFile;

    while (dirIter.hasNext())
    {
        dirIter.next();
        if (QFileInfo(dirIter.filePath()).isFile())
        {

            curFile = dirIter.filePath();
          //  qDebug() << curFile;
            _fileList.push_back(curFile);

        }
    }
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    char temp[DATA_SIZE];
    int i =0;
    int seqNum = 0;
    packet dgram;
    QFile file(index.data().toString());

    //open the file for reading
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "cannot find file";
        return;
    }

    while ((file.read(temp, DATA_SIZE)))
    {
        float window = file.size()/DATA_SIZE;
        BuildPacket(dgram, 0, seqNum, (int)window, DATA_PACKET, RECEIVER_PORT, temp, (char*)RECV_ADDR);

        // Write the datagram
        WriteUDP(dgram);

        i += DATA_SIZE;
        seqNum++;
        file.seek(i);
        memset(temp, 0, sizeof(temp));
    }

}

void MainWindow::readtxDatagrams()
{
    packet packet;

    while (tx_socket->hasPendingDatagrams())
    {
        tx_socket->readDatagram((char*)&packet, sizeof(packet));
        ProcessPacket(packet);
    }
}

void MainWindow::readrxDatagrams()
{
    packet packet;

    while (rx_socket->hasPendingDatagrams())
    {
        rx_socket->readDatagram((char*)&packet, sizeof(packet));
        ProcessPacket(packet);
    }
}

void MainWindow::WriteUDP(packet p)
{
    tx_socket->writeDatagram( (char*)&p, sizeof(p), QHostAddress(NETWORK_ADDR), NETWORK_PORT);
}


void MainWindow::ProcessPacket(packet p)
{
    PrintPacketInfo(p);

    switch (p.PacketType)
    {
        case CONTROL_PACKET:

            break;
        case DATA_PACKET:
            packet dgram;

            BuildPacket(dgram, p.SeqNum, p.SeqNum, 0, CONTROL_PACKET, TRANSMIT_PORT, (char*)"ACK", (char*)TRANSMIT_ADDR);
            WriteUDP(dgram);
            break;
        default:
            break;
    }
}

 void MainWindow::BuildPacket(packet &p, int ack, int seq, int win, int type, int destPort, char* data, char* destAddr)
 {
     p.AckNum = ack;
     p.PacketType = type;
     sprintf(p.data, "%s", data);
     p.SeqNum = seq;
     p.WindowSize = win;
     sprintf(p.dest_addr, "%s", destAddr);
     p.dest_port = destPort;
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
