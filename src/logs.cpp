#include "logs.h"



void logs::out(int level, QString message) {
    std::cout << "[ENGINE] " << message.toStdString() << std::endl;
}
