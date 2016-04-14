#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QString>
#include "audiomanager.h"

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

    void on_disconnectServerBtn_clicked();

    void on_refreshListBtn_clicked();

    void on_dwldFileBtn_clicked();

    void updateSongList(const QString &s);

private:
    Ui::MainWindow *ui;
    AudioManager *audioManager;
};

#endif // MAINWINDOW_H
