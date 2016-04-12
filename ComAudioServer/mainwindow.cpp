#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"

#include <QDebug>
#include <QFileDialog>
#include <io.h>

CircularBuffer *circularBufferRecv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    circularBufferRecv = new CircularBuffer(CIRCULARBUFFERSIZE, CLIENT_PACKET_SIZE, this);
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startServerBtn_clicked()
{

}

void MainWindow::on_startBroadcastBtn_clicked()
{
    controlSockOpen = ServerReceiveSetup(controlSock, CONTROL_PORT, true);
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
