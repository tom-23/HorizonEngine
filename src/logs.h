#ifndef LOGS_H
#define LOGS_H

#include <QObject>
#include <QDebug>

namespace logs {
    void out(int level, QString message);
}

#endif // LOGS_H
