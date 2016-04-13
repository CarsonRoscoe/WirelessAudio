#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QThread>
#include "audiomanager.h"
#include "populatemicrophoneworker.h"

enum ProgramState { MediaPlayer = 0, FileTransfer = 1, Radio = 2, VoiceChat = 3 };

extern ProgramState CurrentState;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_playPauseBtn_clicked();

    void on_skipFwdBtn_clicked();

    void on_skipBkwdBtn_clicked();

    void on_nextSongBtn_clicked();

    void on_prevSongBtn_clicked();

    void on_resumeBtn_clicked();

    void on_connectPeerBtn_clicked();

    void on_sendFileBtn_clicked();

    void on_requestFileBtn_clicked();

    void on_connectServerBtn_clicked();

    void on_connectPeerVoiceBtn_clicked();

    void on_recordBtn_clicked();

    void on_stopRecordBtn_clicked();

    void on_connectOutBtn_clicked();

    void on_disconnectOutBtn_clicked();

    void on_openInBtn_clicked();

    void on_closeInBtn_clicked();

    void cleanupp2p();

    void on_tabWidget_currentChanged(int index);


private:
    Ui::MainWindow *ui;
    PopulateMicrophoneWorker *microphoneWorker;
    QThread microphoneThread;
    AudioManager *audioManager;

    void get_local_files();
    QString get_selected_list_item(int tab);
};

#endif // MAINWINDOW_H
