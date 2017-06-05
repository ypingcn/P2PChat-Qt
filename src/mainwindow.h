#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

    /* --- Program State for fileServer --- */
    enum CurrentState{SendFile,ReceiveFile,NoState};

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /* --- Tools --- */
    QString toIPv4(quint32 arg);
    QString getLocalIP();
    bool vaildNickName(QString name);
    void showMessage(MessageType type,QString hint,QString content);

    /* --- UDP --- */
    void sendJson(MessageType type,QString nick_name,QString content = "");

private:
    Ui::MainWindow *ui;

    /* --- UDP --- */
    const qint16 DEFAULT_MESSAGE_PORT = 6108;
    QUdpSocket * messageSender,* messageReader;
    /* --- TCP --- */
    quint16 FILE_PORT = 6109;

    QTcpServer * fileServer;
    CurrentState state;

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
    void stopToRecvFile();
    void readConnection();
    void sendConnection();
    void sendFileInfo();
    void continueToSend(qint64 size);
};

#endif // MAINWINDOW_H
