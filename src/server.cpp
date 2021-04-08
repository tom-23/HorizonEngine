#include "server.h"

Server::Server(QObject *parent) : QObject(parent)
{
    QLocalServer::removeServer("HorizonAUMANEngine");

    localServer = new QLocalServer(this);
    connect(localServer, &QLocalServer::newConnection, this, [=] () {
        socket = localServer->nextPendingConnection();
        in.setDevice(socket);
        in.setVersion(QDataStream::Qt_5_10);
        connect(socket, &QLocalSocket::disconnected, this, &Server::socketDisconnected);
        connect(socket, &QLocalSocket::readyRead, this, &Server::readReady);
        logs::out(3, "New connection recieved");
    });

    localServer->listen("HorizonAUMANEngine");
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("status", "listening");
    doc.setObject(obj);
    std::cout << doc.toJson().toStdString();
}

void Server::setAudioManager(AudioManager *_audioManager) {
    audioManager = _audioManager;
    connect(audioManager, &AudioManager::send_stat_message, this, &Server::sendStatMessage);
}

void Server::sendStatMessage(QString message, QJsonValue value0, QJsonValue value1) {
    logs::out(3, "Sending command");
    QJsonObject obj;
    obj.insert("cmnd", message);
    obj.insert("value0", value0);
    qDebug() << value1;
    obj.insert("value1", value1);
    QJsonDocument doc;
    doc.setObject(obj);
    //dataQueue->push_back(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    writeString(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void Server::socketDisconnected() {
    logs::out(3, "Socket disconnected! Quitting...");
    qApp->exit(0);
}

void Server::writeString(QString string) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << string;
    socket->write(block);
    socket->waitForBytesWritten();
    qApp->processEvents();
}

void Server::readReady() {
    logs::out(3, "Received Data");

    c = c + 1;
    qDebug() << "cval" << c;



    while (!in.atEnd()) {
        in.startTransaction();
        QString data;
        in >> data;

        if (!in.commitTransaction()){

            logs::out(3, "Data isn't complete. Waiting for next part...");
                // readyRead will be called again when there is more data
                return;
        }
        recievedCompleteData(data);
        //data = "";
        //in >> data;
    }
}

void Server::recievedCompleteData(QString data) {
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());

    QJsonObject obj = doc.object();

    QString cmnd = obj.value("cmnd").toString();
    logs::out(3, "Got data" + data);
    logs::out(3, "Processing CMND:" + cmnd);
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

    if (cmnd == "setTrackGain") {
        QString uuid = obj.value("value0").toString();
        audioManager->getTrack(uuid)->setGain(obj.value("value1").toDouble());
    }

    if (cmnd == "setTrackPan") {
        QString uuid = obj.value("value0").toString();
        audioManager->getTrack(uuid)->setPan(obj.value("value1").toInt());
    }

    if (cmnd == "setTrackMute") {
        QString uuid = obj.value("value0").toString();
        audioManager->getTrack(uuid)->setMute(obj.value("value1").toBool());
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

    if (cmnd == "setRegionLocation") {
        QString regionUUID = obj.value("value0").toString();
        AudioRegion *audioRegion = audioManager->getAudioRegion(regionUUID);
        audioRegion->setGridLocation(obj.value("value1").toDouble());
    }

    if (cmnd == "loadProject") {
        QJsonObject project = obj.value("value0").toObject();
        audioManager->deSerialize(project);
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
        logs::out(3, "Processed " + cmnd);


    qApp->processEvents();
}

void Server::sendConfirmation() {

    logs::out(3, "Sending result...");
    QJsonObject obj;
    obj.insert("result", "OK");
    QJsonDocument doc;
    doc.setObject(obj);
    dataQueue->push_back(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    if (dataQueue->size() >= 1) {
        //writeString();
    }
}
