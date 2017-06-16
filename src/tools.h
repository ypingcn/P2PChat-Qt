#ifndef TOOLS_H
#define TOOLS_H

#include <QObject>

class Tools : public QObject
{
public:
    Tools();

    /* --- Tools --- */
    static QString toIPv4(quint32 arg);
    static QString getLocalIP();
    static bool vaildNickName(QString name);

};

#endif // TOOLS_H
