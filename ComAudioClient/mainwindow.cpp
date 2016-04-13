#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Client.h"
#include "clientudp.h"
#include "udpthread.h"

#include <QDebug>
#include <QAudioInput>
#include <QIODevice>
#include <QTimer>
#include <io.h>

QFile dFile;
QAudioInput * audio;
CircularBuffer * cb, *circularBufferRecv;
QBuffer *microphoneBuffer, *listeningBuffer;
QString lastSong;
bool isRecording;
bool isPlaying;

QThread* multicastThread;

UDPThread* udp_thread;

ClientUDP udpclient;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    isRecording = false;
    isPlaying = false;
    lastSong == "";
    audioManager = new AudioManager(this);
//    microphoneBuffer = new QBuffer(parent);
    listeningBuffer = new QBuffer(parent);
    audioManager->Init(listeningBuffer);
//    ClientReceiveSetupP2P();
//    ClientListenP2P();

//    circularBufferRecv = new CircularBuffer(CIRCULARBUFFERSIZE, SERVER_PACKET_SIZE, this);
    QRegExp regex;
    regex.setPattern("^(([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))\\.){3}([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))$");
    QValidator* val = new QRegExpValidator(regex, this);

    ui->setupUi(this);

    ui->peerIp->setValidator(val);
    ui->serverIp->setValidator(val);
    ui->peerVoiceIp->setValidator(val);
//    ui->peerIp->setText("192.168.0.6");
//    ui->serverIp->setText("192.168.0.6");
//    ui->peerVoiceIp->setText("192.168.0.6");

    ui->peerIp->setText("127.0.0.1");
    ui->serverIp->setText("127.0.0.1");
    ui->peerVoiceIp->setText("127.0.0.1");

    get_local_files();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete audioManager;
}

QString MainWindow::get_selected_list_item(int tab) {

    if (tab == 0) {
        return ui->songList->currentItem()->text();
    } else {
        return ui->localFileList->currentItem()->text();
    }
}

void MainWindow::get_local_files() {

    ui->localFileList->clear();
    ui->songList->clear();

    QStringList nameFilter("*.wav");

    QDir directory(QDir::currentPath());

    if (!directory.cd("../AudioFiles")) {
        qWarning() << "Can't find /AudioFiles directory!";
    }

    QStringList files = directory.entryList(nameFilter);

    ui->localFileList->addItems(files);
    ui->songList->addItems(files);
    ui->localFileList->setCurrentRow(0);
    ui->songList->setCurrentRow(0);
}

// ---- Media Player Functions ----

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

        QString fileName = get_selected_list_item(0);

        qDebug() << "fileName = " << fileName;
        qDebug() << "lastSong = " << lastSong;

        if (fileName == lastSong) {
            audioManager->resume();
        } else {
            if (lastSong != "") {
                audioManager->stop();
            }
            lastSong = fileName;

            QFile *file = new QFile(dir.absoluteFilePath(fileName));
            audioManager->loadSong(file);
        }

    }
}

void MainWindow::on_resumeBtn_clicked()
{
    audioManager->resume();
}

void MainWindow::on_skipFwdBtn_clicked()
{
    //audioManager->skip(10);
    audioManager->playRecord();
}

void MainWindow::on_skipBkwdBtn_clicked()
{
    audioManager->skip(-10);
}

void MainWindow::on_nextSongBtn_clicked()
{

}

void MainWindow::on_prevSongBtn_clicked()
{

}

// ---- File Transfer Functions ----

void MainWindow::on_connectPeerBtn_clicked()
{

}

void MainWindow::on_sendFileBtn_clicked()
{
    QFile *file = new QFile(QFileDialog::getOpenFileName(this, tr("Pick A Song To Send"), 0, tr("Music (*.wav)")));
    if (file->exists())
    {
        file->open(QIODevice::ReadOnly);
        ClientSend((HANDLE) _get_osfhandle(file->handle()));
    }
}

void MainWindow::on_requestFileBtn_clicked()
{

}

void MainWindow::on_connectOutBtn_clicked()
{
    if (ClientSendSetup(ui->peerIp->text().toLatin1().data()) == 0)
    {
        ui->connectOutBtn->setEnabled(false);
        ui->peerIp->setEnabled(false);
        ui->disconnectOutBtn->setEnabled(true);
        ui->sendFileBtn->setEnabled(true);
    }
}

void MainWindow::on_disconnectOutBtn_clicked()
{
    ClientCleanup();
    ui->connectOutBtn->setEnabled(true);
    ui->peerIp->setEnabled(true);
    ui->disconnectOutBtn->setEnabled(false);
    ui->sendFileBtn->setEnabled(false);
}

void MainWindow::on_openInBtn_clicked()
{
    if (ClientReceiveSetup() == 0)
    {
        QFile *file = new QFile(QFileDialog::getSaveFileName(this, tr("Save song as"), 0, tr("Music (*.wav)")));
        if (file->fileName() != NULL)
        {
            ui->openInBtn->setEnabled(false);
            ui->closeInBtn->setEnabled(true);
            file->open(QIODevice::WriteOnly);
            ClientListen((HANDLE) _get_osfhandle(file->handle()));
        } else
        {
            ClientCleanup();
        }
    }
}

void MainWindow::on_closeInBtn_clicked()
{
    ui->openInBtn->setEnabled(true);
    ui->closeInBtn->setEnabled(false);
    ClientCleanup();
}

// ---- Radio Streaming Functions ----

void MainWindow::on_connectServerBtn_clicked()
{
    multicastThread = new QThread();

    udp_thread = new UDPThread();

    udp_thread->moveToThread(multicastThread);

    //connect(sender, signal, receiver, method, ConnectionType)
    connect(udp_thread, SIGNAL(udp_thread_requested()), multicastThread, SLOT(start()));
    connect(multicastThread, SIGNAL(started()), udp_thread, SLOT(receive()));
    connect(udp_thread, SIGNAL(finished()), multicastThread, SLOT(quit()), Qt::DirectConnection);

    udp_thread->udp_thread_request();
}



// ---- PTP Microphone Chat Functions ----

void MainWindow::on_connectPeerVoiceBtn_clicked()
{
   microphoneBuffer->open( QIODevice::ReadWrite);
   listeningBuffer->open(QIODevice::ReadWrite);
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
   //connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

   //QTimer::singleShot(5000, this, SLOT(on_pushButton_2_clicked()));
   isRecording = true;
   audio->start(microphoneBuffer);

   ClientSendSetupP2P(ui->peerVoiceIp->text().toLatin1().data());
   ClientSendMicrophoneData();
}

void MainWindow::on_recordBtn_clicked()
{
    QIODevice *QID;
    //QID->open( QIODevice::WriteOnly);
    QBuffer myQB;

   //QID(myQB);
  //cb(128000,64000);
   //dFile.setFileName("../RecordTest.raw");
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
   //connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

   //QTimer::singleShot(5000, this, SLOT(on_pushButton_2_clicked()));
   isRecording = true;
   audio->start(microphoneBuffer);
}

void MainWindow::on_stopRecordBtn_clicked()
{
    isRecording = false;
    qDebug()<<"StopRecordTriggered";
    audio->stop();
    audioManager->playRecord();
    //dFile.close();
    //delete audio;
}

