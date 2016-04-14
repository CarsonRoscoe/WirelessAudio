/*---------------------------------------------------------------------------------------------------------------------
-- CLASS FILE: Win32CommunicationWorker.cpp
--
-- PROGRAM: ComAudioClient
--
-- METHODS:
--    void doWork()
--
-- DATE: April 1st 2016
--
-- REVISIONS: April 13th 2016:  Created
--            April 14th, 2016: Commented
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES:
-- Win32 worker QT thread created to allow Win32 threads to set a flag when they want to modify the UI and then signal
-- this thread to do so. This thread will then emit to the main UI thread the update and set the flag back to false.
--
-- The need for this is due to QT having a horrible lack of communication methods with Win32 threads.
---------------------------------------------------------------------------------------------------------------------*/

#include "win32communicationworker.h"
#include "Client.h"

bool startP2PAudio = false;

/*---------------------------------------------------------------------------------------------------------------------
-- METHOD: doWork.cpp
--
-- PROGRAM: ComAudioClient
--
-- METHODS:
--    void doWork()
--
-- DATE: April 1st 2016
--
-- REVISIONS: April 13th 2016:  Created
--            April 14th, 2016: Commented
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES:
-- The working logic for the thread.
---------------------------------------------------------------------------------------------------------------------*/
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
