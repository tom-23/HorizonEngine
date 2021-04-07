#include <QCoreApplication>
#include "server.h"
#include "audiomanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server *server = new Server(&a);
    server->setAudioManager(new AudioManager(&a));
    return a.exec();
}
