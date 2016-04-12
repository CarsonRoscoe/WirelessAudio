#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    void on_startServerBtn_clicked();

    void on_startBroadcastBtn_clicked();

    void on_prevSongBtn_clicked();

    void on_playPauseBtn_clicked();

    void on_nextSongBtn_clicked();

    void on_sendFileBtn_clicked();

    void on_connectOutBtn_clicked();

    void on_disconnectOutBtn_clicked();

    void on_openInBtn_clicked();

    void on_closeInBtn_clicked();

private:

    void load_local_files();

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
