#include "logs.h"



void logs::out(int level, QString message) {
    qDebug() << "[INFO]" << message;
}
