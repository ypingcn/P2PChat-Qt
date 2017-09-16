#ifndef FILEWORKER_H
#define FILEWORKER_H

#include "chatworker.h"


#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>

class fileWorker : public QObject
{
    Q_OBJECT

public:

    explicit fileWorker(QObject *parent = 0);

    enum listen_t{LT_LISTEN,LT_UNLISTEN}; // 正在监听、未监听
    enum update_t{UT_SHOW,UT_HIDE,UT_SETMAX,UT_SETVALUE}; // 更新进度条状态（显示、隐藏、设置最大值、设置当前数值）

    bool startListen(void);
    void startSend(void);
    void stopWorker(void);

    bool setSendFile(QString path);
    void setArgs(QString ip,QString port);

    listen_t status();

private:

    listen_t currentListenType; // 程序对于接收端口的监听状态

    QString IP,PORT;
    const QString DEFAULT_FILE_STORE = "D:\\"; // 默认保存到D盘（Windows端有效）

    QTcpServer * fileServer;

    /* --- File Send --- */
    QString filePath; // 所要发送的文件路径信息
    qint8 sendTimes; // 文件发送次数。第一次尝试连接相应地址，后续发送文件信息
    QTcpSocket * sendSocket; // 文件发送socket
    QFile * sendFile; // 将要发送的文件
    QString sendFileName; // 将要发送的文件名
    qint64 sendFileTotalSize,sendFileLeftSize,sendFileEachTransSize; //发送文件总大小、剩余大小、每次传输块大小
    QByteArray sendFileBlock; // 每次传输的文件内容

    /* --- File Receive --- */
    QTcpSocket * receiveSocket; // 文件接收socket
    QFile * receiveFile; // 接收的文件
    QString receiveFileName; // 接受的文件名
    qint64 receiveFileTotalSize,receiveFileTransSize; // 接收文件总大小、接收文件已传输大小
    QByteArray receiveFileBlock; // 每次接收的文件内容

signals:

    void messageShowReady(chatWorker::message_t type, QString hint, QString content);
    void progressBarUpdateReady(fileWorker::update_t type, qint64 number);

private slots:

    void acceptConnection();
    void readConnection();
    void sendFileInfo();
    void continueToSend(qint64 size);
};

#endif // FILEWORKER_H
