#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Client.h"

#include <QDebug>
#include <QAudioInput>
#include <QIODevice>
#include <QTimer>
#include <io.h>

QFile dFile;
QAudioInput * audio;
CircularBuffer * cb, *circularBufferRecv;
QBuffer *microphoneBuffer, *listeningBuffer;
bool isRecording, isPlaying;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    isRecording = false;
    isPlaying = false;
    audioManager = new AudioManager(this);
    microphoneBuffer = new QBuffer(parent);
    listeningBuffer = new QBuffer(parent);
    audioManager->Init(listeningBuffer);
    //p2pListenSockClosed = ClientReceiveSetup(p2pListenSock, P2P_DEFAULT_PORT, p2pAcceptEvent);
    //ClientListenP2P();

    circularBufferRecv = new CircularBuffer(CIRCULARBUFFERSIZE, SERVER_PACKET_SIZE, this);
    QRegExp regex;
    regex.setPattern("^(([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))\\.){3}([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))$");
    QValidator* val = new QRegExpValidator(regex, this);

    ui->setupUi(this);

    ui->peerIp->setValidator(val);
    ui->serverIp->setValidator(val);
    ui->peerVoiceIp->setValidator(val);
    ui->peerIp->setText("192.168.0.5");
    ui->serverIp->setText("192.168.0.5");
    ui->peerVoiceIp->setText("127.0.0.1");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete audioManager;
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

        //audioManager->stop();
        QFile *file = new QFile(QFileDialog::getOpenFileName(this, tr("Pick A Song"), 0, tr("Music (*.wav)")));
        audioManager->loadSong(file);
    }
}

void MainWindow::on_resumeBtn_clicked()
{
    audioManager->resume();
}

void MainWindow::on_skipFwdBtn_clicked()
{
    audioManager->skip(10);
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

void MainWindow::on_requestFileBtn_clicked()
{

}

// ---- Radio Streaming Functions ----

void MainWindow::on_connectServerBtn_clicked()
{
    if ((controlSockClosed = ClientSendSetup(ui->serverIp->text().toLatin1().data(),
            controlSock, CONTROL_PORT)) == 0)
    {
        ui->connectServerBtn->setEnabled(false);
        ui->serverIp->setEnabled(false);
        ui->disconnectServerBtn->setEnabled(true);
        ui->refreshListBtn->setEnabled(true);
        ui->sendFileBtn->setEnabled(true);
        ui->dwldFileBtn->setEnabled(true);
    }
}

void MainWindow::on_disconnectServerBtn_clicked()
{
    ClientCleanup();
    ui->connectServerBtn->setEnabled(true);
    ui->serverIp->setEnabled(true);
    ui->disconnectServerBtn->setEnabled(false);
    ui->refreshListBtn->setEnabled(false);
    ui->sendFileBtn->setEnabled(false);
    ui->dwldFileBtn->setEnabled(false);
}

void MainWindow::on_refreshListBtn_clicked()
{
    ClientSendRequest(GET_UPDATE_SONG_LIST);
}

void MainWindow::on_sendFileBtn_clicked()
{
    QFile *file = new QFile(QFileDialog::getOpenFileName(this, tr("Pick A Song To Send"), 0, tr("Music (*.wav)")));
    if (file->exists())
    {
        file->open(QIODevice::ReadOnly);
    }
    QFileInfo fileInfo(file->fileName());
    strcpy(sendFileName, fileInfo.fileName().toLatin1().data());
    ClientSendRequest(SEND_SONG_TO_SERVER);
}

void MainWindow::on_dwldFileBtn_clicked()
{

}

// ---- PTP Microphone Chat Functions ----

void MainWindow::on_connectPeerVoiceBtn_clicked()
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
   audioManager->playRecord();

   p2pSendSockClosed = ClientSendSetup(ui->peerVoiceIp->text().toLatin1().data(),
                                        p2pSendSock, P2P_DEFAULT_PORT);
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
