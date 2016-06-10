#ifndef WIN32COMMUNICATIONWORKER_H
#define WIN32COMMUNICATIONWORKER_H

#include <QBuffer>
#include <QDebug>

extern bool startP2PAudio;

class Win32CommunicationWorker : public QObject
{
    Q_OBJECT
public:
    Win32CommunicationWorker(){}

public slots:
    void doWork();

signals:
    void playLoad();
};


#endif // WIN32COMMUNICATIONWORKER_H
