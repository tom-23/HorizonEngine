#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>

#include "audiomanager.h"
#include "track.h"
#include "audioregion.h"

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    AudioManager *audioManager;
    void setAudioManager(AudioManager *_audioManager);
private:
    QLocalServer *localServer;
    QLocalSocket *socket;
    QDataStream dataStream;
    int connectionCount = 0;
    quint32 blockSize = 0;

    int c = 0;

    void writeString(QString string);
    QList<QString> *dataQueue = new QList<QString>;

    void readReady();

    void recievedCompleteData(QString data);

    void socketDisconnected();

    void sendConfirmation();

    void sendStatMessage(QString message, QJsonValue value0, QJsonValue value1);

    QString data;

    QDataStream in;

    QByteArray inBlock;

signals:

};

#endif // SERVER_H
