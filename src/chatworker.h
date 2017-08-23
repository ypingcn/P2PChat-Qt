#ifndef CHATWORKER_H
#define CHATWORKER_H

#include <QObject>
#include <QUdpSocket>

class chatWorker : public QObject
{
    Q_OBJECT

public:

    explicit chatWorker(QObject *parent = 0);

    enum MessageType{Chat,Login,Logout,Involved,System};  //聊天、登录、退出、在线、系统信息
    enum StatusType{Online,Offline};

    void setMask(QString mask) { MASK = mask; }
    void setStatus(StatusType status) { currentStatusType = status; }
    void setUserName(QString name) { USER_NAME = name; }

    StatusType status() { return currentStatusType; }

    void sendJson(MessageType type, QString nick_name, QString content="");

private:

    QString MASK;
    QString USER_NAME;

    StatusType currentStatusType;

    QSet<QString> onlineUsersSet;

    QUdpSocket * messageSender,* messageReader;
    const qint16 DEFAULT_MESSAGE_PORT = 6108; // 默认信息端口

signals:

    void messageShowReady(chatWorker::MessageType type, QString hint, QString content);
    void onlineUsersUpdateReady(QSet<QString> set);

private slots:

    void readAllMessage();

};

#endif // CHATWORKER_H
