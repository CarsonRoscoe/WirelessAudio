#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Client.h"

#include <QDebug>
#include <QAudioInput>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>
#include <io.h>
#include <QPalette>
#include <QDataStream>

QFile dFile;
QAudioInput * audio;
QPalette palette;

//QAudioInput * audioFile;
CircularBuffer  *circularBufferRecv, *micBuf;
QBuffer *microphoneBuffer, *listeningBuffer;
bool isRecording;
bool isPlaying;
QByteArray byteArray;
int curpos=0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    isRecording = false;
    isPlaying = false;
    microphoneBuffer = new QBuffer(parent);
    listeningBuffer = new QBuffer(parent);
    listeningBuffer->open(QIODevice::ReadWrite);
    micBuf=new CircularBuffer(CIRCULARBUFFERSIZE, SERVER_PACKET_SIZE, this);
    audioManager = new AudioManager(this);
    circularBufferRecv = new CircularBuffer(CIRCULARBUFFERSIZE, SERVER_PACKET_SIZE, this);
    audioManager->Init(listeningBuffer, circularBufferRecv);
    if (ClientReceiveSetupP2P() != -1)
        ClientListenP2P();
    else
        qDebug() << "ClientReceiveSetupP2P Error'd";

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
    ui->peerIp->setText("192.168.0.7");
    ui->serverIp->setText("127.0.0.7");
    ui->peerVoiceIp->setText("192.168.0.7");

    microphoneWorker = new PopulateMicrophoneWorker(micBuf, microphoneBuffer);
    microphoneWorker->moveToThread(&microphoneThread);
    connect(&microphoneThread, &QThread::finished, microphoneWorker, &QObject::deleteLater);
    connect(&microphoneThread, SIGNAL(started()), microphoneWorker, SLOT(doWork()));
    microphoneThread.start();
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

}

// ---- PTP Microphone Chat Functions ----

void MainWindow::on_connectPeerVoiceBtn_clicked()
{
   // byteArray=microphoneBuffer->buffer();
    //dFile.setFileName("../RecordTest.raw");
    dFile.open( QIODevice::ReadWrite);

   microphoneBuffer->buffer().reserve(10000000);
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

   //audioFile = new QAudioInput(format, this);
   audio = new QAudioInput(format, this);
   //audio->setBufferSize(SERVER_PACKET_SIZE * BUFFERSIZE);
   //connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

   //QTimer::singleShot(5000, this, SLOT(on_pushButton_2_clicked()));
   isRecording = true;
   //connect(audio,SIGNAL(notify()),this,SLOT(StoreToBuffer()));
   audio->start(microphoneBuffer);
   //audioFile->start(&dFile);
   ClientSendSetupP2P(ui->peerVoiceIp->text().toLatin1().data());
   ClientSendMicrophoneData();
}

void MainWindow::on_recordBtn_clicked()
{


   //byteArray=microphoneBuffer->buffer();
   dFile.setFileName("../RecordTest.raw");
   dFile.open( QIODevice::ReadWrite);
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

   //audioFile = new QAudioInput(format, this);
   audio = new QAudioInput(format, this);
    audio->setNotifyInterval(1);
   //connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
   connect(audio,SIGNAL(notify()),this,SLOT(StoreToBuffer()));
   //QTimer::singleShot(5000, this, SLOT(on_pushButton_2_clicked()));
   isRecording = true;
   audio->start(microphoneBuffer);
  // if(packetcounter>10)

   //audioFile->start(&dFile);

}

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

void MainWindow::cleanupp2p()
{

        qDebug()<<"cleanup";
}
/*void MainWindow::StoreToBuffer(){
    char tempbuff[CLIENT_PACKET_SIZE];
    //char *tempbuff = (char *)malloc(CLIENT_PACKET_SIZE);

    microphoneBuffer->seek(curpos);
    if(microphoneBuffer->bytesAvailable()>=CLIENT_PACKET_SIZE){
        int bytes= microphoneBuffer->read(tempbuff, CLIENT_PACKET_SIZE);
        curpos+=bytes;
        microphoneBuffer->seek(microphoneBuffer->size()-1);
        qDebug() << "Bytes Available: " << microphoneBuffer->bytesAvailable();
        qDebug()<<"Push back to buffer her, bytes read:" << bytes;

        if(!micBuf->pushBack(tempbuff)){
            qDebug()<<"Pushback FAILED";
        }
    }
}*/
