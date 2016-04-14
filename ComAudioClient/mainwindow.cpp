/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: mainwindow.cpp - This file handles interaction with the UI.
--
-- PROGRAM: comaudio
--
-- FUNCTIONS:
--  MainWindow
-- ~MainWindow()
-- get_selected_list_item(int tab)
-- on_playPauseBtn_clicked()
-- on_skipFwdBtn_clicked()
-- on_skipBkwdBtn_clicked()
-- on_nextSongBtn_clicked()
-- on_prevSongBtn_clicked()
-- on_connectPeerBtn_clicked()
-- on_requestFileBtn_clicked()
-- void MainWindow::on_connectServerBtn_clicked()
-- void MainWindow::on_disconnectServerBtn_clicked()
-- void MainWindow::connect_to_radio()
-- void MainWindow::play_incoming_stream()
-- void MainWindow::on_refreshListBtn_clicked()
-- void MainWindow::on_sendFileBtn_clicked()
-- void MainWindow::on_dwldFileBtn_clicked()
-- void MainWindow::on_connectPeerVoiceBtn_clicked()
-- void MainWindow::on_recordBtn_clicked()
-- void MainWindow::on_stopRecordBtn_clicked()
-- void MainWindow::updateSongList(const QString &s)
-- void MainWindow::cleanupp2p()
-- void MainWindow::on_tabWidget_currentChanged(int index)
-- void MainWindow::on_volumeSlider_sliderMoved(int position)

-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee, Thomas Yu, Carson Roscoe, Micah Williams
--
-- PROGRAMMER: Spenser Lee, Thomas Yu, Carson Roscoe, Micah Williams
--
-- NOTES:
-- This file is responsible for linking together the functionality of the program with a user interface.
--

----------------------------------------------------------------------------------------------------------------------*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Client.h"
#include "udpthread.h"
#include "clientudp.h"
#include "populatemicrophoneworker.h"
#include "wavheader.h"

#include <QDebug>
#include <QAudioInput>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>
#include <io.h>
#include <QPalette>
#include <QDataStream>

QFile *file;
QFile dFile;
QAudioInput * audio = NULL;
QPalette palette;

//QAudioInput * audioFile;
CircularBuffer  *circularBufferRecv, *micBuf;
QBuffer *microphoneBuffer, *listeningBuffer;
bool isRecording, isPlaying;
bool playing = false;
QString lastSong;
QByteArray byteArray;
int curpos=0;
AudioManager *audioManager = NULL;
ProgramState CurrentState = MediaPlayer;

QThread* multicastThread;
UDPThread* udp_thread;

ClientUDP udpclient;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MainWindow(QWidget *parent)
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee,Carson Roscoe, Thomas Yu
--
-- INTERFACE: MainWindow(QWidget *parent)
--
-- RETURNS: N/A - constructor
--
-- NOTES:
-- This is the constructor for the main window.
----------------------------------------------------------------------------------------------------------------------*/
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

    isRecording = false;
    isPlaying = false;
    lastSong == "";

    microphoneBuffer = new QBuffer(parent);
    microphoneBuffer->buffer().reserve(10000000);
    listeningBuffer = new QBuffer(parent);
    listeningBuffer->buffer().reserve(10000000);
    listeningBuffer->open(QIODevice::ReadWrite);

    micBuf=new CircularBuffer(CIRCULARBUFFERSIZE, SERVER_PACKET_SIZE, this);
    audioManager = new AudioManager(this);
    circularBufferRecv = new CircularBuffer(200, SERVER_PACKET_SIZE, this);
    audioManager->Init(listeningBuffer, circularBufferRecv);

    QRegExp regex;
    regex.setPattern("^(([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))\\.){3}([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))$");
    QValidator* val = new QRegExpValidator(regex, this);

    ui->setupUi(this);

    ui->serverIp->setValidator(val);
    ui->peerVoiceIp->setValidator(val);
    ui->serverIp->setText("192.168.0.5");
    ui->peerVoiceIp->setText("192.168.0.7");

    get_local_files();
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MainWindow()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: MainWindow()
--
-- RETURNS: N/A - destructor
--
-- NOTES:
-- This is the destructor for the main window.
----------------------------------------------------------------------------------------------------------------------*/
MainWindow::~MainWindow()
{
    delete ui;
    delete audioManager;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: get_selected_list_item(int tab)
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: MainWindow(QWidget *parent)
--
-- RETURNS: QString - song name
--
-- NOTES:
-- This functions returns a QString of the selected song.
----------------------------------------------------------------------------------------------------------------------*/
QString MainWindow::get_selected_list_item() {

    return ui->songList->currentItem()->text();
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: get_local_files()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: get_local_files()
--
-- RETURNS: N/A - constructor
--
-- NOTES:
-- grabs the local files that can be played.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::get_local_files() {

    ui->songList->clear();

    QStringList nameFilter("*.wav");

    QDir directory(QDir::currentPath());

    if (!directory.cd("../AudioFiles")) {
        qWarning() << "Can't find /AudioFiles directory!";
    }

    QStringList files = directory.entryList(nameFilter);

    ui->songList->addItems(files);
    ui->songList->setCurrentRow(0);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_playPauseBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: on_playPauseBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the play/pause button.
----------------------------------------------------------------------------------------------------------------------*/

void MainWindow::on_playPauseBtn_clicked()
{
    if (isPlaying) {    // pause music

        ui->playPauseBtn->setText("Play");
        isPlaying = false;

        audioManager->pause();

    } else {            // play music

        ui->playPauseBtn->setText("Pause");
        isPlaying = true;

        QDir dir(QDir::currentPath());
        if (!dir.cd("../AudioFiles")) {
            qWarning() << "Can't find /AudioFiles directory!";
        }

        QString fileName = get_selected_list_item();

        qDebug() << "fileName = " << fileName;
        qDebug() << "lastSong = " << lastSong;

        if (fileName == lastSong) {
            audioManager->resume();
        } else {
            if (lastSong != "") {
                audioManager->stop();
            }
            lastSong = fileName;

            qDebug() << dir.absoluteFilePath(fileName);

            QFile *file = new QFile(dir.absoluteFilePath(fileName));
            audioManager->loadSong(file);
        }

    }
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_resumeBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: on_resumeBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the resume button.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_resumeBtn_clicked()
{
    //audioManager->resume();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_skipFwdBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:on_skipFwdBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the skip forward button
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_skipFwdBtn_clicked()
{
    audioManager->skip(10);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_skipBkwBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:on_skipBckBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the skip back button
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_skipBkwdBtn_clicked()
{
    audioManager->skip(-10);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_nextSongBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:on_nextSongBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the play next song button.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_nextSongBtn_clicked()
{
 //audio->reset(microphoneBuffer);
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_prevSongBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:on_prevSongBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the play previous song button.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_prevSongBtn_clicked()
{

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_connectServerBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:on_connectServerBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the connect to radio server to start receiving the broadcast.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_connectServerBtn_clicked()
{
//    if ((controlSockClosed = ClientSendSetup(ui->serverIp->text().toLatin1().data(),
//            controlSock, CONTROL_PORT)) == 0)
//    {
//        strcpy(address, ui->serverIp->text().toLatin1().data());
        ui->connectServerBtn->setEnabled(false);
        ui->serverIp->setEnabled(false);
        ui->disconnectServerBtn->setEnabled(true);
        ui->sendFileBtn->setEnabled(true);
        ui->dwldFileBtn->setEnabled(true);
//    }
        QTimer::singleShot(5000, this, SLOT(on_refreshListBtn_clicked()));

    connect_to_radio();
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_disconnectServerBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:on_disconnectServerBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This is the disconnect to radio server to stop receiving the broadcast.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_disconnectServerBtn_clicked()
{
//    closesocket(controlSock);
//    ClientCleanup();
    ui->connectServerBtn->setEnabled(true);
    ui->serverIp->setEnabled(true);
    ui->disconnectServerBtn->setEnabled(false);
    ui->sendFileBtn->setEnabled(false);
    ui->dwldFileBtn->setEnabled(false);

    udp_thread->close_socket();
    audioManager->pause();
    circularBufferRecv->resetBuffer();
    playing = false;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connect_to_radio()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:connect_to_radio()
--
-- RETURNS: void
--
-- NOTES:
-- This is the connect to radio server to start receiving the broadcast.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::connect_to_radio() {
    multicastThread = new QThread();

    udp_thread = new UDPThread();

    udp_thread->moveToThread(multicastThread);

    // connect(sender, signal, receiver, method, ConnectionType)
    connect(udp_thread, SIGNAL(udp_thread_requested()), multicastThread, SLOT(start()), Qt::UniqueConnection);
    connect(multicastThread, SIGNAL(started()), udp_thread, SLOT(receive()), Qt::UniqueConnection);
    connect(udp_thread, SIGNAL(stream_data_recv()), this, SLOT(play_incoming_stream()), Qt::UniqueConnection);

    udp_thread->udp_thread_request();
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connect_to_radio()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:connect_to_radio()
--
-- RETURNS: void
--
-- NOTES:
-- Play broadcast
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::play_incoming_stream() {
    if (!playing) {

        audioManager->play();
        playing = true;
    }
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connect_to_radio()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:connect_to_radio()
--
-- RETURNS: void
--
-- NOTES:
-- Refresh song list
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_refreshListBtn_clicked()
{
    ClientSendRequest(GET_UPDATE_SONG_LIST);
    QTimer::singleShot(5000, this, SLOT(on_refreshListBtn_clicked()));
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_sendFileBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Micah Williams
--
-- PROGRAMMER: Micah Williams
--
-- INTERFACE:on_sendFileBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- Sends a song to the server.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_sendFileBtn_clicked()
{
    QFile *file = new QFile(QFileDialog::getOpenFileName(this, tr("Pick A Song To Send"), 0, tr("Music (*.wav)")));
    if (file->exists())
    {
        file->open(QIODevice::ReadOnly);
        hSendFile = (HANDLE)_get_osfhandle(file->handle());
        QFileInfo fileInfo(file->fileName());
        strcpy(sendFileName, fileInfo.fileName().toLatin1().data());
        ClientSendRequest(SEND_SONG_TO_SERVER);
    }
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_sendFileBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Micah Williams
--
-- PROGRAMMER: Micah Williams
--
-- INTERFACE:on_sendFileBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- To download a song from the server.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_dwldFileBtn_clicked()
{
    ClientSendRequest(GET_SONG_FROM_SERVER);
}

// ---- PTP Microphone Chat Functions ----
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_connectPeerVoiceBtn_clicked(
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe, Thomas Yu
--
-- PROGRAMMER: Carson Roscoe, Thomas Yu
--
-- INTERFACE:on_connectPeerVoiceBtn_clicked(
--
-- RETURNS: void
--
-- NOTES:
-- This function is called when the connect button is clicked. It starts a thread to populate a circular buffer and
-- initializes microphone recording and sets up a connection to another client.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_connectPeerVoiceBtn_clicked()
{
    microphoneWorker = new PopulateMicrophoneWorker(micBuf, microphoneBuffer);
    microphoneWorker->moveToThread(&microphoneThread);
    connect(&microphoneThread, &QThread::finished, microphoneWorker, &QObject::deleteLater);
    connect(&microphoneThread, SIGNAL(started()), microphoneWorker, SLOT(doWork()));
    microphoneThread.start();
   microphoneBuffer->open( QIODevice::ReadWrite);
   QAudioFormat format;
   // Set up the desired format, for example:
   format.setSampleRate(15000);
   format.setChannelCount(1);
   format.setSampleSize(16);
   format.setCodec("audio/pcm");
   format.setByteOrder(QAudioFormat::LittleEndian);
   format.setSampleType(QAudioFormat::UnSignedInt);

   QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
   if (!info.isFormatSupported(format))
   {
       qWarning() << "Default format not supported, trying to use the nearest.";
       format = info.nearestFormat(format);
   }

   audio = new QAudioInput(format, this);
   isRecording = true;
   audio->start(microphoneBuffer);

   ClientSendSetupP2P(ui->peerVoiceIp->text().toLatin1().data());
   ClientSendMicrophoneData();
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_connectPeerVoiceBtn_clicked(
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe, Thomas Yu
--
-- PROGRAMMER: Carson Roscoe, Thomas Yu
--
-- INTERFACE:on_connectPeerVoiceBtn_clicked(
--
-- RETURNS: void
--
-- NOTES:
-- This function starts the recording from a microphone locally.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_recordBtn_clicked()
{

   microphoneBuffer->open( QIODevice::ReadWrite);
   QAudioFormat format;
   // Set up the desired format, for example:
   format.setSampleRate(16000);
   format.setChannelCount(1);
   format.setSampleSize(16);
   format.setCodec("audio/pcm");
   format.setByteOrder(QAudioFormat::LittleEndian);
   format.setSampleType(QAudioFormat::UnSignedInt);

   QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
   if (!info.isFormatSupported(format))
   {
       qWarning() << "Default format not supported, trying to use the nearest.";
       format = info.nearestFormat(format);
   }

   audio = new QAudioInput(format, this);
   microphoneBuffer->buffer().resize(0);
   microphoneBuffer->reset();
   microphoneBuffer->setBuffer(new QByteArray());
   microphoneBuffer->close();
   microphoneBuffer->open(QIODevice::ReadWrite);
   isRecording = true;
   audio->start(microphoneBuffer);
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_stopRecordBtn_clicked()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe, Thomas Yu
--
-- PROGRAMMER: Carson Roscoe, Thomas Yu
--
-- INTERFACE:on_stopRecordBtn_clicked()
--
-- RETURNS: void
--
-- NOTES:
-- This function stops the microphone recording.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_stopRecordBtn_clicked()
{
    isRecording = false;
    qDebug()<<"StopRecordTriggered";
    audio->stop();
    //audioFile->stop();
    audioManager->playRecord();
    //dFile.close();
    //delete audio;

}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateSongList()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE:updateSongList()
--
-- RETURNS: void
--
-- NOTES:
-- This function updates the song list.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::updateSongList(const QString &s)
{
    ui->songList->addItem(s);
}
void MainWindow::cleanupp2p()
{

        qDebug()<<"cleanup";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_tabWidget_currentChanged(int index)
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe, Thomas Yu
--
-- PROGRAMMER: Carson Roscoe, Thomas Yu
--
-- INTERFACE: on_tabWidget_currentChanged(int index)
--          int index: the tab changed into.
-- RETURNS: void
--
-- NOTES:
-- This function disconnects and does cleanup on the appropriate tab switchs
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch(CurrentState) {
    case MediaPlayer:
        //Invoke MediaPlayer cleanup
        break;
     case FileTransfer:
        //Invoke FileTransfer cleanup
        break;
     case Radio:
        //Invoke Radio cleanup
        udp_thread->close_socket();
        audioManager->pause();
        circularBufferRecv->resetBuffer();
        playing = false;
        break;
     case VoiceChat:
        //Invoke VoiceChat cleanup
        CleanupRecvP2P();
        isRecording = false;
        if (audio != NULL) {
            CleanupSendP2P();
            audio->reset();
            audio->stop();
            delete audio;
            audio = NULL;
            micSendPacket = 0;
            micPos = 0;
        }
        listeningBuffer->buffer().resize(0);
        listeningBuffer->reset();
        listeningBuffer->close();
        listeningBuffer->open(QIODevice::ReadWrite);
        break;
    default:
        qDebug()<<"This state should never happen?";
    }

    CurrentState = static_cast<ProgramState>(index);

    switch(CurrentState) {
        case VoiceChat:
            if (ClientReceiveSetupP2P() != -1) {
                ClientListenP2P();
                qDebug() << "Started up P2P listening";
            } else {
                qDebug() << "Could not start P2P listening";
            }

            break;
    }
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_tabWidget_currentChanged(int index)
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Thomas Yu
--
-- PROGRAMMER: Thomas Yu
--
-- INTERFACE: on_tabWidget_currentChanged(int index)
--          int index: the tab changed into.
-- RETURNS: void
--
-- NOTES:
-- This function disconnects and does cleanup on the appropriate tab switchs
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_volumeSlider_sliderMoved(int position)
{
    double temp = position;
    double dVol=temp/99;
    qDebug()<<"current vol val:"<<position<<"dvol:"<<dVol;
    audioManager->setVolume(dVol);
}
