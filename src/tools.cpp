#include "tools.h"

#include <QRegExp>
#include <QNetworkInterface>

Tools::Tools()
{

}

QString Tools::toIPv4(quint32 arg)
{
    QString res;
    int bits[4],cnt = 0;
    while (arg)
    {
        bits[cnt] = arg % 256;
        arg /= 256;
        cnt++;
    }
    for(int i=3;i>=0;i--)
        res.append(QString::number(bits[i]).append(i==0?"":"."));
    return res;
}

QString Tools::getLocalIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach(QHostAddress in,list)
    {
        if(in.protocol() == QAbstractSocket::IPv4Protocol && in.toString().contains("192.168"))
            return in.toString();
    }
    return QString::null;
}

bool Tools::vaildNickName(QString name)
{
    QRegExp reg1("[A-Za-z0-9_]{1,}");
    QRegExp reg2("[\u4E00-\u9FA5]{1,}");
    if(reg1.exactMatch(name))
        return true;
    if(reg2.exactMatch(name))
        return true;
    return false;
}
