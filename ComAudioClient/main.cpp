#include "mainwindow.h"
#include "Client.h"
#include <QApplication>
#include <QListWidget>

/*---------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Main.cpp
--
-- PROGRAM: ComAudioClient
--
-- FUNCTIONS:
--    int main(int argc, char *argv[])
--
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- NOTES:
-- Entry point for the program.
---------------------------------------------------------------------------------------------------------------------*/

void *app;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    QListWidget *lw = w.findChild<QListWidget*>("songList");

    return a.exec();

}
