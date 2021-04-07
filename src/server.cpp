#include "server.h"

Server::Server(QObject *parent) : QObject(parent)
{
    QLocalServer::removeServer("HorizonAUMANEngine");

    localServer = new QLocalServer(this);
    connect(localServer, &QLocalServer::newConnection, this, [=] () {
        socket = localServer->nextPendingConnection();
        dataStream.setDevice(socket);
        dataStream.setVersion(QDataStream::Qt_5_10);
        connect(socket, &QLocalSocket::disconnected, this, &Server::socketDisconnected);
        connect(socket, &QLocalSocket::readyRead, this, &Server::readReady);
        qDebug() << "New Connection recieved!";
    });

    localServer->listen("HorizonAUMANEngine");

}

void Server::setAudioManager(AudioManager *_audioManager) {
    audioManager = _audioManager;
    connect(audioManager, &AudioManager::send_stat_message, this, &Server::sendStatMessage);
}

void Server::sendStatMessage(QString message, QJsonValue value0, QJsonValue value1) {
    qDebug() << "Sending stat message";
    QJsonObject obj;
    obj.insert("cmnd", message);
    obj.insert("value0", value0);
    qDebug() << value1;
    obj.insert("value1", value1);
    QJsonDocument doc;
    doc.setObject(obj);
    dataQueue->push_back(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    if (dataQueue->size() == 1) {
        writeString();
    }
}

void Server::socketDisconnected() {
    qDebug() << "Socket disconnected";
}

void Server::writeString() {
    if (dataQueue->size() == 0) {
        return;
    }
    //string = "TESTING";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    const QString &message = dataQueue->at(0);
    out << quint32(message.size());
    out << message;
    socket->write(block);
    socket->flush();
}

void Server::readReady() {
    qDebug() << "Received Data";

    if (blockSize == 0) {
        // Relies on the fact that QDataStream serializes a quint32 into
        // sizeof(quint32) bytes
        if (socket->bytesAvailable() < (int) sizeof(quint32)) { return; };
        dataStream >> blockSize;
    }


    if (socket->bytesAvailable() < blockSize || dataStream.atEnd()) {
        return;
    }
    QString data;
    dataStream >> data;
    blockSize = 0;
    qDebug() << data;
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());

    QJsonObject obj = doc.object();
    if (!obj.value("result").isUndefined()) {
        qDebug() << "Got result";
        if (doc.object().value("result").toString() == "OK") {
            if (dataQueue->size() != 0) {
                dataQueue->pop_front();
                writeString();
                socket->flush();
            }
        }
        return;
    }
    QString cmnd = obj.value("cmnd").toString();
    qDebug() << "Processing CMND:" << cmnd;
    qDebug() << data;
    if (cmnd == "init") {
        audioManager->initContext();
    }

    if (cmnd == "setBPM") {
        double bpm = obj.value("value0").toDouble();
        audioManager->setBPM(bpm);
    }

    if (cmnd == "setDivision") {
        double division = obj.value("value0").toDouble();
        audioManager->setDivision(division);
    }

    if (cmnd == "addTrack") {
        QString uuid = obj.value("value0").toString();
        audioManager->addTrack(uuid);
    }

    if (cmnd == "addAudioRegion") {
        QString trackUUID = obj.value("value0").toString();
        QString regionUUID = obj.value("value1").toString();
        Track *track = audioManager->getTrack(trackUUID);
        track->addAudioRegion(regionUUID);
    }

    if (cmnd == "loadAudioRegion") {
        QString regionUUID = obj.value("value0").toString();
        QString fileName = obj.value("value1").toString();
        AudioRegion *audioRegion = audioManager->getAudioRegion(regionUUID);
        audioRegion->loadFile(fileName);
    }

    if (cmnd == "play") {
        audioManager->play();
    }

    if (cmnd == "pause") {
        audioManager->pause();
    }

    if (cmnd == "stop") {
        audioManager->stop();
    }
    socket->flush();
    sendConfirmation();
}


void Server::sendConfirmation() {
    qDebug() << "Sending confirmation";
    QJsonObject obj;
    obj.insert("result", "OK");
    QJsonDocument doc;
    doc.setObject(obj);
    dataQueue->push_back(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    if (dataQueue->size() == 1) {
        writeString();
    }
}