#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"

#include "serverudp.h"
#include "readfileworker.h"
#include "../ComAudioClient/circularbuffer.h"

#include <QDebug>
#include <QFileDialog>
#include <io.h>

/*---------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: MainWindow.cpp
--
-- PROGRAM: ComAudioServer
--
-- FUNCTIONS:
--   MainWindow(QWidget *parent = 0);
--   void on_startServerBtn_clicked();
--   void on_startBroadcastBtn_clicked();
--   void on_prevSongBtn_clicked();
--   void on_playPauseBtn_clicked();
--   void on_nextSongBtn_clicked();
--   void change_song(QString);
--   void load_local_files();
--   void start_radio();
--   DWORD WINAPI send_thread(LPVOID lp_param);
--
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee & Micah Willems
--
-- PROGRAMMER: Spenser Lee & Micah Willems
--
-- NOTES:
-- This final contains the connecting functions between the UI and the other logic.
---------------------------------------------------------------------------------------------------------------------*/

ServerUDP udpserver;
QThread rfw_thread;

CircularBuffer *circularBufferRecv;
QFile *file;

DWORD WINAPI send_thread(LPVOID lp_param);

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MainWindow (constructor)
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee & Micah Willems
--
-- PROGRAMMER: Spenser Lee & Micah Willems
--
-- INTERFACE: MainWindow(QWidget *parent)
--              parent: parent QWidget handle
--
-- RETURNS: nothing.
--
-- NOTES:
-- Constructor for the MainWindow object.
---------------------------------------------------------------------------------------------------------------------*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QFile f("../sweetbabyjesus.qss");
    if (!f.exists())
    {
        printf("Unable to set stylesheet, file not found\n");
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    circularBufferRecv = new CircularBuffer(CIRCULARBUFFERSIZE, CLIENT_PACKET_SIZE, this);

    ui->setupUi(this);

    load_local_files();
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ~MainWindow (destructor)
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee & Micah Willems
--
-- PROGRAMMER: Spenser Lee & Micah Willems
--
-- INTERFACE: ~MainWindow()
--
-- RETURNS: nothing.
--
-- NOTES:
-- Destructor for the MainWindow object.
---------------------------------------------------------------------------------------------------------------------*/
MainWindow::~MainWindow()
{
    delete ui;
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: load_local_files
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: load_local_files()
--
-- RETURNS: void
--
-- NOTES:
-- Looks for the local files and populates a list widget with their names.
---------------------------------------------------------------------------------------------------------------------*/
void MainWindow::load_local_files() {
    ui->songList->clear();

    QStringList nameFilter("*.wav");
    QDir directory(QDir::currentPath());

//    if (!directory.cd("./Library/")) {
    if (!directory.cd("../AudioFiles/")) {
        qWarning() << "Can't find Library directory!";
        return;
    }

    QStringList files = directory.entryList(nameFilter);

    ui->songList->addItems(files);
    ui->songList->setCurrentRow(0);
}


/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_startServerBtn_clicked
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: on_startServerBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- On click for start server button.
---------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_startServerBtn_clicked()
{

}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_startBroadcastBtn_clicked
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: on_startBroadcastBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- On click for start broadcast button.
---------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_startBroadcastBtn_clicked()
{
//    controlSockOpen = ServerReceiveSetup(controlSock, CONTROL_PORT, true);
    start_radio();
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: start_radio
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: start_radio()
--
-- RETURNS: void
--
-- NOTES:
-- Initializes the UDP multicast socket and launches a ReadFileWorker thread.
---------------------------------------------------------------------------------------------------------------------*/
void MainWindow::start_radio() {
    if (!udpserver.init_socket(MULTICAST_PORT)) {
        qWarning() << "failed to init socket";
    }

    if (!udpserver.init_multicast(MULTICAST_IP)) {
        qWarning() << "failed to set multicast settings";
    }

    QDir dir(QDir::currentPath());
    if (!dir.cd("../AudioFiles")) {
        qWarning() << "Can't find /AudioFiles directory!";
    }

    QString fileName = ui->songList->currentItem()->text();

    file = new QFile(dir.absoluteFilePath(fileName));
    ReadFileWorker *rfw = new ReadFileWorker(file, circularBufferRecv);

    rfw->moveToThread(&rfw_thread);
    connect(&rfw_thread, &QThread::finished, rfw, &QObject::deleteLater);
    connect(&rfw_thread, SIGNAL(started()), rfw, SLOT(doWork()));

    connect(this, SIGNAL(change_song(QString)), rfw, SLOT(load_song(QString)), Qt::DirectConnection);

    rfw_thread.start();

    DWORD thread_id;
    if (CreateThread(NULL, 0, send_thread, (LPVOID) &udpserver, 0, &thread_id) == NULL) {
        qWarning() << "failed to create thread";
    }
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: send_thread
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: DWORD WINAPI send_thread(LPVOID lp_param)
--                  lp_param: thread parameters
--
-- RETURNS: DWORD
--
-- NOTES:
-- Thread for broadcasting data from the circular buffer.
---------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI send_thread(LPVOID lp_param) {
    qDebug() << "thread created";

    ServerUDP* serv = (ServerUDP*)lp_param;
    DWORD bytes_to_send = SERVER_PACKET_SIZE;

    char message[SERVER_PACKET_SIZE] = { '\0' };
    memset(message, '\0', sizeof(message));
    int bytesPerSecond = 44100 * 16 / 8 * 2;
    int bytesRead = 0;
    while (1) {
        if (!circularBufferRecv->pop(message)) {
            continue;
        }
        if (!serv->broadcast_message(message, &bytes_to_send)) {
            qWarning() << "broadcast failed";
        }
        bytesRead += bytes_to_send;
        if (bytesRead < bytesPerSecond)
            continue;
        SleepEx(300, true);
        bytesRead = 0;
    }
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_nextSongBtn_clicked
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: on_nextSongBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- On click for for next song button. Used for queuing the next song for the stream.
---------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_queueSongBtn_clicked()
{
    QString filename = ui->songList->currentItem()->text();
    QDir dir(QDir::currentPath());
    if (!dir.cd("../AudioFiles")) {
        qWarning() << "Can't find /AudioFiles directory!";
    }

    emit change_song(dir.absoluteFilePath(filename));
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_prevSongBtn_clicked
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: on_prevSongBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- On click for start broadcast button.
---------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_prevSongBtn_clicked()
{

}
