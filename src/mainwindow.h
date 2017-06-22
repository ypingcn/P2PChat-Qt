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

    /* --- Message Type for showMessage() --- */
    enum MessageType{Chat,Login,Logout,Online,System};
    /* --- Local Listen Type --- */
    enum ListenType{Listen,Unlisten};

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    /* --- Tools ---*/
    void sendJson(MessageType type,QString nick_name,QString content = "");
    void showMessage(MessageType type,QString hint,QString content);
    bool localUserStatus();
    void setLocalUserStatus(bool status);
    void setLocalFileStatus(bool status);

    /* --- UDP --- */
    const qint16 DEFAULT_MESSAGE_PORT = 6108;
    const qint8 DEFAULT_MESSAGE_FONT_SIZE = 14;
    QUdpSocket * messageSender,* messageReader;
    /* --- TCP --- */
    const QString DEFAULT_FILE_IP = "127.0.0.1";
    const quint16 DEFAULT_FILE_PORT = 6109;

    QTcpServer * fileServer;

    /* --- File Send --- */
    qint8 sendTimes;
    QTcpSocket * sendSocket;
    QFile * sendFile;
    QString sendFileName;
    qint64 sendFileTotalSize,sendFileLeftSize,sendFileEachTransSize;
    QByteArray sendFileBlock;

    /* --- File Receive --- */
    QTcpSocket * receiveSocket;
    QFile * receiveFile,* finalFile;
    QString receiveFileName;
    qint64 receiveFileTotalSize,receiveFileTransSize;
    QByteArray receiveFileBlock;
    ListenType listenType;

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
