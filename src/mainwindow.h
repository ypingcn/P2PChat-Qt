#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tools.h"
#include "fileworker.h"
#include "chatworker.h"
#include "hintwidget.h"

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum WidgetState{Initial,Add,Remove};

    fileWorker * file;
    chatWorker * chat;

private:
    Ui::MainWindow *ui;
    hintWidget * hint;

    void setLocalUserEnable(bool status); // 改变用户登录状态
    void setLocalFileEnable(bool status); // 改变文件传输按钮可用性
    void setLanguage(void); // 设置界面语言
    void setTheme(void); // 设置程序主题
    void setWidgetState(WidgetState state);
    void getHelp(void);// 帮助界面

    const qint8 DEFAULT_MESSAGE_FONT_SIZE = 14; // 默认信息显示大小

    const QString DEFAULT_FILE_IP = "127.0.0.1"; // 默认文件传输IP地址（内置的地址为演示用）
    const quint16 DEFAULT_FILE_PORT = 6109; // 默认文件传输端口

private slots:

    void showMessage(chatWorker::message_t type,QString hint,QString content); // 将接收到或自身的信息显示到内容框内
    void updateProgressBar(fileWorker::update_t type, qint64 number);
    void updateOnlineUsers(QSet<QString> set);

    void click_btnSendMessage();
    void click_btnLogout();
    void click_btnLogin();

    void click_btnChooseFile();
    void click_btnListen();
    void click_btnSendFile();

    void updateFinalIP(QListWidgetItem*);
};

#endif // MAINWINDOW_H
