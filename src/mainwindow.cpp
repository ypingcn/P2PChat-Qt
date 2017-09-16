#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QSettings>
#include <QFileDialog>
#include <QDesktopServices>
#include <QPropertyAnimation>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    hint = new hintWidget(ui->browserMessage);
    hint->resize(ui->browserMessage->width(),32);
    hint->hide();


    connect(ui->actionEnglish,&QAction::triggered,this,&MainWindow::setLanguage);
    connect(ui->actionSimplifiedChinese,&QAction::triggered,this,&MainWindow::setLanguage);
    connect(ui->actionTraditionalChinese,&QAction::triggered,this,&MainWindow::setLanguage);

    connect(ui->actionHelp,&QAction::triggered,this,&MainWindow::getHelp);
    connect(ui->actionAbout,&QAction::triggered,this,&MainWindow::getHelp);

    connect(ui->btnLogin,SIGNAL(clicked()),this,SLOT(click_btnLogin()));
    connect(ui->btnLogout,SIGNAL(clicked(bool)),this,SLOT(click_btnLogout()));
    connect(ui->btnListen,SIGNAL(clicked(bool)),this,SLOT(click_btnListen()));
    connect(ui->btnSendFile,SIGNAL(clicked(bool)),this,SLOT(click_btnSendFile()));
    connect(ui->btnChooseFile,SIGNAL(clicked(bool)),this,SLOT(click_btnChooseFile()));
    connect(ui->btnSendMessage,SIGNAL(clicked()),this,SLOT(click_btnSendMessage()));

    ui->labIPAdress->setText(Tools::getLocalIP());
    ui->edtFinalIP->setText(DEFAULT_FILE_IP);
    ui->edtFinalPort->setText(QString::number(DEFAULT_FILE_PORT));
    ui->ProgressBar->hide();

    setLocalUserEnable(false);
    setWidgetState(Initial);

    QFont font;
    font.setPixelSize(DEFAULT_MESSAGE_FONT_SIZE);
    ui->browserMessage->setFont(font);

    file = new fileWorker;
    chat = new chatWorker;

    connect(file,SIGNAL(messageShowReady(chatWorker::message_t,QString,QString)),
            this,SLOT(showMessage(chatWorker::message_t,QString,QString)) );
    connect(file,SIGNAL(progressBarUpdateReady(fileWorker::update_t,qint64)),
            this,SLOT(updateProgressBar(fileWorker::update_t,qint64)) );
    connect(chat,SIGNAL(messageShowReady(chatWorker::message_t,QString,QString)),
            this,SLOT(showMessage(chatWorker::message_t,QString,QString)) );
    connect(chat,SIGNAL(onlineUsersUpdateReady(QSet<QString>)),
            this,SLOT(updateOnlineUsers(QSet<QString>)) );
}

MainWindow::~MainWindow()
{
    chat->sendJson(chatWorker::MT_LOGOUT,ui->edtName->text()); // 销毁对象前先退出
    delete ui;
}
void MainWindow::setLanguage()
{
    QSettings settings("ypingcn","p2pchat-qt");

    if(QObject::sender() == ui->actionEnglish)
        settings.setValue("p2pchat-qt-lang","eng");
    else if(QObject::sender() == ui->actionSimplifiedChinese)
        settings.setValue("p2pchat-qt-lang","zh-cn");
    else if(QObject::sender() == ui->actionTraditionalChinese)
        settings.setValue("p2pchat-qt-lang","zh-tw");

    hint->setText(tr("Restart the app to switch language"));
}

void MainWindow::getHelp()
{
    QDialog * help = new QDialog(this);
    QVBoxLayout * helpLayout = new QVBoxLayout(help);

    if(QObject::sender() == ui->actionAbout)
    {
        help->setWindowTitle(tr("About"));
        QLabel * content = new QLabel();
        QString website = "<a href=\"https://github.com/ypingcn/P2PChat-Qt\">" + tr("click to know more.") + "</a>";
        content->setText(tr("A simple program designed for LAN chat.")+website);
        content->setOpenExternalLinks(true);
        helpLayout->addWidget(content);
    }
    else if(QObject::sender() == ui->actionHelp)
    {
        help->setWindowTitle(tr("Help"));
        QTextEdit * content = new QTextBrowser();
        help->resize(600,380);
        content->append(tr("P2PChat-Qt Help"));
        content->append(tr("\n --- Nick Name Help"));
        content->append(tr("Valid character include : numbers , alphabet ( lower-case and upper-case ) , underline and simplified chinese character."));
        content->append(tr("\n --- Mask Help"));
        content->append(tr("Mask help you to chat in different range."));
        content->append(tr("#255.255.255.255 option# local usage , Presentation mode"));
        content->append(tr("#192.168.1.1 option# LAN usage , if two clients whose IP begins with 192.168 , are in the same LAN"));
        content->append(tr("\n --- IP Args Help"));
        content->append(tr("two number mean IP address and port number."));
        content->append(tr("You should input an vaild IP address in the first blank ( four numbers, separated by dots and ranged between 0 and 255 )"));
        content->append(tr("#127.0.0.1 is Presentation mode, you should input the destination IP instead.( As OnlineUsers shows )"));
        content->append(tr("Number in the second blank should be a number ranged between 1 and 65535"));
        content->append(tr("Port number don't recommand to change unless the two clients use the same port for file transmisson."));
        content->append(tr("'Listen' means I am ready to get the file."));
        helpLayout->addWidget(content);
    }

    help->exec();

}

void MainWindow::showMessage(chatWorker::message_t type, QString hint, QString content)
{
    QDateTime now = QDateTime::currentDateTime();
    if(type == chatWorker::MT_LOGIN || type == chatWorker::MT_LOGOUT || type == chatWorker::MT_SYSTEM)
    {
        ui->browserMessage->setTextColor(QColor(190,190,190));
        ui->browserMessage->append(hint + now.toString("  hh:mm:ss"));
        ui->browserMessage->append(content);
    }
    else if(type == chatWorker::MT_CHAT)
    {
        ui->browserMessage->setTextColor(QColor(70,130,180));
        ui->browserMessage->append(hint + now.toString("  hh:mm:ss"));
        ui->browserMessage->setTextColor(QColor(0,0,0));
        ui->browserMessage->append(content);
    }
}

void MainWindow::updateProgressBar(fileWorker::update_t type,qint64 number)
{
    if(type == fileWorker::UT_SHOW)
        ui->ProgressBar->show();
    else if(type == fileWorker::UT_HIDE)
        ui->ProgressBar->hide();
    else if(type == fileWorker::UT_SETVALUE)
        ui->ProgressBar->setValue(number);
    else if(type == fileWorker::UT_SETMAX)
        ui->ProgressBar->setMaximum(number);
}

void MainWindow::updateOnlineUsers(QSet<QString> set)
{
    ui->listOnlineUser->clear();
    ui->listOnlineUser->insertItem(0,tr("Online Users:"));
    int i = 1;
    for(auto it = set.begin();it!= set.end() ;it++)
        ui->listOnlineUser->insertItem(i++,*it);
}

void MainWindow::setLocalUserEnable(bool status)
{
    /* --- 根据用户状态设置按钮可用性 ---*/
    ui->edtName->setEnabled(!status);
    ui->edtMessage->setEnabled(status);
    ui->btnSendMessage->setEnabled(status);
    ui->btnLogin->setEnabled(!status);
    ui->btnLogout->setEnabled(status);
    ui->boxMask->setEnabled(!status);
}

void MainWindow::setWidgetState(WidgetState state)
{
    QPropertyAnimation * left = new QPropertyAnimation(ui->listOnlineUser,"size");
    QPropertyAnimation * right = new QPropertyAnimation(ui->browserMessage,"size");
    left->setDuration(200);
    right->setDuration(200);
    left->setEasingCurve(QEasingCurve::OutQuart);
    right->setEasingCurve(QEasingCurve::OutQuart);

    if(state == Add)
    {
        left->setStartValue(QSize(ui->listOnlineUser->width(),ui->listOnlineUser->height()));
        left->setEndValue(QSize(ui->listOnlineUser->width(),ui->listOnlineUser->height()-140));
        right->setStartValue(QSize(ui->browserMessage->width(),ui->browserMessage->height()));
        right->setEndValue(QSize(ui->browserMessage->width(),ui->browserMessage->height()-140));

        left->start();
        right->start();

        ui->labFinalIP->show();
        ui->edtFinalIP->show();
        ui->edtFinalPort->show();
        ui->btnListen->show();
        ui->btnChooseFile->show();
        ui->btnSendFile->show();
        ui->edtMessage->show();
        ui->btnSendMessage->show();
    }
    else if(state == Remove || state == Initial)
    {
        left->setStartValue(QSize(ui->listOnlineUser->width(),ui->listOnlineUser->height()));
        left->setEndValue(QSize(ui->listOnlineUser->width(),ui->listOnlineUser->height()+140));
        right->setStartValue(QSize(ui->browserMessage->width(),ui->browserMessage->height()));
        right->setEndValue(QSize(ui->browserMessage->width(),ui->browserMessage->height()+140));

        left->start();
        right->start();

        ui->labFinalIP->hide();
        ui->edtFinalIP->hide();
        ui->edtFinalPort->hide();
        ui->btnListen->hide();
        ui->btnChooseFile->hide();
        ui->btnSendFile->hide();
        ui->edtMessage->hide();
        ui->btnSendMessage->hide();
    }
}

void MainWindow::click_btnSendMessage()
{
    if(ui->edtMessage->toPlainText().isEmpty())
    {
        hint->setText(tr("No message"));
    }
    else
    {
        chat->setMask(ui->boxMask->currentText());
        chat->sendJson(chatWorker::MT_CHAT,ui->edtName->text(),ui->edtMessage->toPlainText());
        ui->edtMessage->clear();
    }
}

void MainWindow::click_btnLogin()
{
    if(!Tools::vaildNickName(ui->edtName->text()))
    {
        hint->setText(tr("Invaild Nickname!"));
    }
    else if(Tools::getLocalIP() == QString::null)
    {
        hint->setText(tr("Check your Network to login"));
    }
    else
    {
        setLocalUserEnable(true);
        setWidgetState(Add);
        ui->btnListen->setEnabled(true);
        ui->labIPAdress->setText(Tools::getLocalIP());
        chat->setMask(ui->boxMask->currentText());
        chat->sendJson(chatWorker::MT_LOGIN,ui->edtName->text());
        chat->setStatus(chatWorker::ST_OFFLINE);
        chat->setUserName(ui->edtName->text());
    }
}

void MainWindow::click_btnLogout()
{
    if(Tools::getLocalIP() == QString::null)
    {
        hint->setText(tr("Check your Network to logout"));
    }
    else
    {
        setLocalUserEnable(false);
        setWidgetState(Remove);

        chat->setMask(ui->boxMask->currentText());
        chat->sendJson(chatWorker::MT_LOGOUT,ui->edtName->text());
        chat->setStatus(chatWorker::ST_OFFLINE);
    }

}

void MainWindow::click_btnChooseFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose File"), ".", tr("All File(*.*)"));
    if(!path.isEmpty())
    {
        if( file->setSendFile(path) )
            ui->btnSendFile->setEnabled(true);
    }
}

void MainWindow::click_btnListen()
{
    fileWorker::listen_t listenType = file->status();

    if(listenType == fileWorker::LT_UNLISTEN)
    {
        file->setArgs(ui->edtFinalIP->text(), ui->edtFinalPort->text());
        if( file->startListen() )
        {
            ui->btnListen->setText(tr("UnListen"));
            showMessage(chatWorker::MT_SYSTEM,tr("System"),tr(" -- File Port Listening"));
        }
        else
        {
            showMessage(chatWorker::MT_SYSTEM,tr("System"),tr(" -- File Port Listening Fail"));
        }

    }
    else if(listenType == fileWorker::LT_LISTEN)
    {
        ui->btnListen->setText(tr("Listen"));
        file->stopWorker();
        showMessage(chatWorker::MT_SYSTEM,tr("System"),tr(" -- File Port Listening Closed"));
    }
}

void MainWindow::click_btnSendFile()
{
    file->setArgs(ui->edtFinalIP->text(),ui->edtFinalPort->text());
    file->startSend();
}

