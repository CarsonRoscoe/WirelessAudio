#include "mainwindow.h"
#include <QApplication>

/*---------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Main.cpp
--
-- PROGRAM: ComAudioServer
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
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
