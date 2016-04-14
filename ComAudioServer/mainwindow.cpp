#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"

#include "serverudp.h"
#include "readfileworker.h"
#include "../ComAudioClient/circularbuffer.h"

#include <QDebug>
#include <QFileDialog>
#include <io.h>

ServerUDP udpserver;

CircularBuffer *circularBufferRecv;
CircularBuffer *cb;

DWORD WINAPI send_thread(LPVOID lp_param);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    circularBufferRecv = new CircularBuffer(CIRCULARBUFFERSIZE, CLIENT_PACKET_SIZE, this);

    ui->setupUi(this);

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

QThread rfw_thread;

void MainWindow::on_startServerBtn_clicked()
{
    if (!udpserver.init_socket(4985)) {
        qDebug() << "failed to init socket";
    }

    if (!udpserver.init_multicast("234.5.6.7")) {
        qDebug() << "failed to set multicast settings";
    }

    cb = new CircularBuffer(100, SERVER_PACKET_SIZE, this);

    QDir dir(QDir::currentPath());
    if (!dir.cd("../AudioFiles")) {
        qWarning() << "Can't find /AudioFiles directory!";
    }

    QString fileName = ui->songList->currentItem()->text();

    QFile *file = new QFile(dir.absoluteFilePath(fileName));
    ReadFileWorker *rfw = new ReadFileWorker(file, cb);

    rfw->moveToThread(&rfw_thread);
    connect(&rfw_thread, &QThread::finished, rfw, &QObject::deleteLater);
    connect(&rfw_thread, SIGNAL(started()), rfw, SLOT(doWork()));

    rfw_thread.start();

    DWORD thread_id;
    if (CreateThread(NULL, 0, send_thread, (LPVOID) &udpserver, 0, &thread_id) == NULL) {
        qDebug() << "failed to create thread";
    }
}

DWORD WINAPI send_thread(LPVOID lp_param) {
    qDebug() << "thread created";

    ServerUDP* serv = (ServerUDP*)lp_param;
    DWORD bytes_to_send = SERVER_PACKET_SIZE;
    DWORD total = 0;

    char message[SERVER_PACKET_SIZE] = { '\0' };
    memset(message, '\0', sizeof(message));

//    QString path = QCoreApplication::applicationDirPath();
//    path.append("/testing.wav");
//    QFile myfile(path);

//    if(!myfile.open(QIODevice::ReadWrite)) {
//        qDebug() << "failed";
//    }

    while (1) {


        if (!cb->pop(message)) {
            qDebug() << "couldn't pop off cb";
            continue;
        }

//        if (myfile.write(message, MY_BUF_SIZE) < 0) {
//            qDebug() << "error writing file";
//        }

        if (!serv->broadcast_message(message, &bytes_to_send)) {
            qDebug() << "broadcast failed";
        }

    }
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
