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

    void on_queueSongBtn_clicked();

signals:
    void change_song(QString);

private:

    void load_local_files();
    void start_radio();

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
