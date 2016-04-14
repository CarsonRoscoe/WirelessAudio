#include "win32communicationworker.h"
#include "Client.h"

bool startP2PAudio = false;

void Win32CommunicationWorker::doWork() {
    while(true) {
        if (startP2PAudio) {
            qDebug() << "Sending emit";

            emit playLoad();

            startP2PAudio = false;
        }
        SleepEx(25, true);
    }
}
