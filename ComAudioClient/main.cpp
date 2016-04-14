#include "mainwindow.h"
#include "Client.h"
#include <QApplication>
#include <QListWidget>

void *app;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    QListWidget *lw = w.findChild<QListWidget*>("songList");

    return a.exec();
}
