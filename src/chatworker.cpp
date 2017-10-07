#include "chatworker.h"
#include "tools.h"

#include <QJsonObject>
#include <QJsonDocument>

chatWorker::chatWorker(QObject *parent) : QObject(parent)
{
    messageSender = new QUdpSocket();
    messageReader = new QUdpSocket();

    messageReader->bind(DEFAULT_MESSAGE_PORT,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint );
    connect(messageReader,SIGNAL(readyRead()),this,SLOT(readAllMessage()));
}

void chatWorker::sendJson(message_t type, QString nick_name, QString content)
{
    QJsonObject obj;

    if(nick_name.isEmpty())
        return;

    if(type == MT_CHAT)
        obj.insert("type","chat");
    else if(type == MT_LOGIN)
        obj.insert("type","login");
    else if(type == MT_LOGOUT)
        obj.insert("type","logout");
    else if(type == MT_INVOLVE)
        obj.insert("type","involved");


    if( !content.isEmpty() ) // content 默认为空
        obj.insert("content",content);

    obj.insert("nick-name",nick_name);

    QJsonDocument doc; // json 格式封装将要发送到聊天室内的信息
    doc.setObject(obj);

    QByteArray data = doc.toJson();

    messageSender->writeDatagram(data.data(),data.size(),QHostAddress(MASK),DEFAULT_MESSAGE_PORT);
}

void chatWorker::readAllMessage()
{
    while (messageReader->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(messageReader->pendingDatagramSize());
        QHostAddress source; // 信息来源IP
        messageReader->readDatagram(data.data(),data.size(),&source);

        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(data,&jsonError);
        if(jsonError.error == QJsonParseError::NoError && doc.isObject())
        {
            QJsonObject obj = doc.object();
            if(obj.contains("type") && obj.contains("nick-name"))
            {
                QJsonValue type = obj.take("type"); // 信息类型
                QString info = obj.take("nick-name").toString() + "(" + Tools::toIPv4(source.toIPv4Address()) + ")" ;
                if(type.toString() == "chat" && obj.contains("content"))
                {
                    emit messageShowReady(chatWorker::MT_CHAT,info,obj.take("content").toString());
                }
                else if(type.toString() == "login")
                {
                    /* --- 查找同名、同IP的用户信息 不存在则添加--- */
                    if( !onlineUsersSet.contains(info) )
                        onlineUsersSet.insert(info);
                    if( currentStatusType == ST_ONLINE) // 本地用户在线则向聊天室发送 Involved 信息，交换在线用户信息
                        sendJson(chatWorker::MT_INVOLVE,USER_NAME);
                    emit messageShowReady(chatWorker::MT_LOGIN,info,tr(" -- enter the chat room"));
                }
                else if(type.toString() == "logout")
                {
                    /* --- 查找退出用户信息并从列表删除 --- */
                    if(onlineUsersSet.contains(info))
                        onlineUsersSet.remove(info);
                    emit messageShowReady(chatWorker::MT_LOGOUT,info,tr(" -- quit the chat room"));
                }
                else if(type.toString() == "involved")
                {
                    /* --- 更新在线用户信息 --- */
                    if(!onlineUsersSet.contains(info))
                        onlineUsersSet.insert(info);
                }
            }
            emit onlineUsersUpdateReady(onlineUsersSet);
        }
    }

}
