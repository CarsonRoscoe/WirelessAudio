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
QAudioInput * audio = NULL;
QPalette palette;

//QAudioInput * audioFile;
CircularBuffer  *circularBufferRecv, *micBuf;
QBuffer *microphoneBuffer, *listeningBuffer;
bool isRecording, isPlaying;
QString lastSong;
QByteArray byteArray;
int curpos=0;
AudioManager *audioManager;
ProgramState CurrentState = MediaPlayer;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    isRecording = false;
    isPlaying = false;
    lastSong == "";
    microphoneBuffer = new QBuffer(parent);
    microphoneBuffer->buffer().reserve(10000000);
    listeningBuffer = new QBuffer(parent);
    //p2pListenSockClosed = ClientReceiveSetup(p2pListenSock, P2P_DEFAULT_PORT, p2pAcceptEvent);
    //ClientListenP2P();
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
    ui->peerIp->setText("192.168.0.7");
    ui->serverIp->setText("192.168.0.5");
    ui->peerVoiceIp->setText("192.168.0.7");

    microphoneWorker = new PopulateMicrophoneWorker(micBuf, microphoneBuffer);
    microphoneWorker->moveToThread(&microphoneThread);
    connect(&microphoneThread, &QThread::finished, microphoneWorker, &QObject::deleteLater);
    connect(&microphoneThread, SIGNAL(started()), microphoneWorker, SLOT(doWork()));
    microphoneThread.start();

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

            qDebug() << dir.absoluteFilePath(fileName);

            QFile *file = new QFile(dir.absoluteFilePath(fileName));
            audioManager->loadSong(file);
        }

    }
}

void MainWindow::on_resumeBtn_clicked()
{
    //audioManager->resume();
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
 //audio->reset(microphoneBuffer);
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
        strcpy(address, ui->serverIp->text().toLatin1().data());
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
    closesocket(controlSock);
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
        hSendFile = (HANDLE)_get_osfhandle(file->handle());
        QFileInfo fileInfo(file->fileName());
        strcpy(sendFileName, fileInfo.fileName().toLatin1().data());
        ClientSendRequest(SEND_SONG_TO_SERVER);
    }
}

void MainWindow::on_dwldFileBtn_clicked()
{
    ClientSendRequest(GET_SONG_FROM_SERVER);
}

// ---- PTP Microphone Chat Functions ----

void MainWindow::on_connectPeerVoiceBtn_clicked()
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

   //audioFile = new QAudioInput(format, this);
   audio = new QAudioInput(format, this);
   //audio->setBufferSize(SERVER_PACKET_SIZE * BUFFERSIZE);
   //connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

   //QTimer::singleShot(5000, this, SLOT(on_pushButton_2_clicked()));
   isRecording = true;
   //connect(audio,SIGNAL(notify()),this,SLOT(StoreToBuffer()));
   audio->start(microphoneBuffer);
   audioManager->playRecord();

   p2pSendSockClosed = ClientSendSetup(ui->peerVoiceIp->text().toLatin1().data(),
                                        p2pSendSock, P2P_DEFAULT_PORT);
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

void MainWindow::updateSongList(const QString &s)
{
    ui->songList->addItem(s);
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

void MainWindow::on_tabWidget_currentChanged(int index)
{
    //enum ProgramState { MediaPlayer = 0, FileTransfer = 1, Radio = 2, VoiceChat = 3 };
    switch(CurrentState) {
    case MediaPlayer:
        //Invoke MediaPlayer cleanup
        break;
     case FileTransfer:
        //Invoke FileTransfer cleanup
        break;
     case Radio:
        //Invoke Radio cleanup
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
        }
       /* listeningBuffer->
        listeningBuffer->reset();
        listeningBuffer->close();
        listeningBuffer->open(QIODevice::ReadWrite);
        */
        ClientReceiveSetupP2P();
        ClientListenP2P();

        break;
    default:
        qDebug()<<"This state should never happen?";
    }

    CurrentState = static_cast<ProgramState>(index);
}
