#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"

#include "serverudp.h"

#include <QDebug>
#include <QFileDialog>
#include <io.h>

ServerUDP udpserver;

CircularBuffer *circularBufferRecv;

DWORD WINAPI send_thread(LPVOID lp_param);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    circularBufferRecv = new CircularBuffer(CIRCULARBUFFERSIZE, CLIENT_PACKET_SIZE, this);
    QRegExp regex;
    regex.setPattern("^(([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))\\.){3}([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))$");
    QValidator* val = new QRegExpValidator(regex, this);
    ui->setupUi(this);
    ui->ipAddr->setValidator(val);
//    ui->ipAddr->setText("192.168.0.5");
    ui->ipAddr->setText("127.0.0.1");

    load_local_files();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::load_local_files() {
    ui->songList->clear();

    QStringList nameFilter("*.wav");
    QDir directory(QDir::currentPath());

    if (!directory.cd("../AudioFiles")) {
        qWarning() << "Can't find /AudioFiles directory!";
        return;
    }

    QStringList files = directory.entryList(nameFilter);

    ui->songList->addItems(files);
    ui->songList->setCurrentRow(0);
}

void MainWindow::on_startServerBtn_clicked()
{
    if (!udpserver.init_socket(7000)) {
        qDebug() << "failed to init socket";
    }

    if (!udpserver.init_multicast("234.5.6.7")) {
        qDebug() << "failed to set multicast settings";
    }

    DWORD thread_id;
    if (CreateThread(NULL, 0, send_thread, (LPVOID) &udpserver, 0, &thread_id) == NULL) {
        qDebug() << "failed to create thread";
    }
}

DWORD WINAPI send_thread(LPVOID lp_param) {
    qDebug() << "thread created";

    ServerUDP* serv = (ServerUDP*)lp_param;
    DWORD bytes_to_send = 10;

    char ** msg = (char**) malloc(sizeof(char*));

    *msg = "123456789";

    while (1) {
        // if there is stuff to send

        if (!serv->broadcast_message(*msg, &bytes_to_send)) {
            qDebug() << "broadcast failed";
        }
        Sleep(1000);
    }
}
void MainWindow::on_startBroadcastBtn_clicked()
{

}

void MainWindow::on_prevSongBtn_clicked()
{

}

void MainWindow::on_playPauseBtn_clicked()
{

}

void MainWindow::on_nextSongBtn_clicked()
{

}

void MainWindow::on_sendFileBtn_clicked()
{
    QFile *file = new QFile(QFileDialog::getOpenFileName(this, tr("Pick a Song to Send"), 0, tr("Music (*.wav)")));
    if (file->exists())
    {
        file->open(QIODevice::ReadOnly);
        ServerSend((HANDLE) _get_osfhandle(file->handle()));
    }
}

void MainWindow::on_connectOutBtn_clicked()
{
    if (ServerSendSetup(ui->ipAddr->text().toLatin1().data()) == 0)
    {
        ui->connectOutBtn->setEnabled(false);
        ui->disconnectOutBtn->setEnabled(true);
        ui->sendFileBtn->setEnabled(true);
    }
}

void MainWindow::on_disconnectOutBtn_clicked()
{
    ServerCleanup();
    ui->connectOutBtn->setEnabled(true);
    ui->disconnectOutBtn->setEnabled(false);
    ui->sendFileBtn->setEnabled(false);
}

void MainWindow::on_openInBtn_clicked()
{
    if (ServerReceiveSetup() == 0)
    {
        QFile *file = new QFile(QFileDialog::getSaveFileName(this, tr("Pick The Destination Song Name"), 0, tr("Music (*.wav)")));
        if (file->fileName() != NULL)
        {
            ui->openInBtn->setEnabled(false);
            ui->closeInBtn->setEnabled(true);
            file->open(QIODevice::WriteOnly);
            ServerListen((HANDLE) _get_osfhandle(file->handle()));
        } else
        {
            ServerCleanup();
        }
    }
}

void MainWindow::on_closeInBtn_clicked()
{
    ui->openInBtn->setEnabled(true);
    ui->closeInBtn->setEnabled(false);
    ServerCleanup();
}
