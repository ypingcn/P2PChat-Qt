#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tools.h"

#include <QMainWindow>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    /* --- Message Type for showMessage() and sendJson()--- */
    enum MessageType{Chat,Login,Logout,Online,System};  //聊天、登录、退出、在线、系统信息
    /* --- Local Listen Type --- */
    enum ListenType{Listen,Unlisten}; // 正在监听、未监听

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    /* --- Tools ---*/
    void sendJson(MessageType type,QString nick_name,QString content = ""); // 向聊天室发送信息
    void showMessage(MessageType type,QString hint,QString content); // 将接收到或自身的信息显示到内容框内
    bool localUserStatus(); // 本地用户登录状态
    void setLocalUserStatus(bool status); // 改变用户登录状态
    void setLocalFileStatus(bool status); // 改变文件传输按钮可用性

    /* --- UDP --- */
    const qint16 DEFAULT_MESSAGE_PORT = 6108; // 默认信息端口
    const qint8 DEFAULT_MESSAGE_FONT_SIZE = 14; // 默认信息显示大小
    QUdpSocket * messageSender,* messageReader;
    /* --- TCP --- */
    const QString DEFAULT_FILE_IP = "127.0.0.1"; // 默认文件传输IP地址（内置的地址为演示用）
    const quint16 DEFAULT_FILE_PORT = 6109; // 默认文件传输端口
    const QString DEFAULT_FILE_STORE = "D:\\"; // 默认保存到D盘（Windows端有效）
    QTcpServer * fileServer;

    /* --- File Send --- */
    QString chooseFileName; // 所要发送的文件路径信息
    qint8 sendTimes; // 文件发送次数。第一次发送文件信息，后续发送文件内容
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
    ListenType listenType; // 程序对于接收端口的监听状态

private slots:

    /* --- UDP --- */
    void readAllMessage();
    void sendChatMessage();
    void sendLoginMessage();
    void sendLogoutMessage();

    /* --- TCP --- */
    void chooseSendFile();
    void listen();
    void acceptConnection();
    void readConnection();
    void sendConnection();
    void sendFileInfo();
    void continueToSend(qint64 size);
};

#endif // MAINWINDOW_H
