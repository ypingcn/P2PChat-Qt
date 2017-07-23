#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->ProgressBar->hide();

    ui->labIPAdress->setText(Tools::getLocalIP());
    ui->edtFinalIP->setText(DEFAULT_FILE_IP);
    ui->edtFinalPort->setText(QString::number(DEFAULT_FILE_PORT));

    setLocalUserStatus(false);
    setLocalFileStatus(false);

    sendTimes = 0;

    QFont font;
    font.setPixelSize(DEFAULT_MESSAGE_FONT_SIZE);
    ui->browserMessage->setFont(font);

    messageSender = new QUdpSocket();
    messageReader = new QUdpSocket();

    messageReader->bind(DEFAULT_MESSAGE_PORT,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint );
    connect(messageReader,SIGNAL(readyRead()),this,SLOT(readAllMessage()));

    connect(ui->btnLogin,SIGNAL(clicked()),this,SLOT(sendLoginMessage()));
    connect(ui->btnLogout,SIGNAL(clicked(bool)),this,SLOT(sendLogoutMessage()));
    connect(ui->btnSendMessage,SIGNAL(clicked()),this,SLOT(sendChatMessage()));
    connect(ui->btnChooseFile,SIGNAL(clicked(bool)),this,SLOT(chooseSendFile()));

    connect(ui->btnListen,SIGNAL(clicked(bool)),this,SLOT(listen()));
    connect(ui->btnSendFile,SIGNAL(clicked(bool)),this,SLOT(sendConnection()));

    fileServer = new QTcpServer();
    connect(fileServer,SIGNAL(newConnection()),this,SLOT(acceptConnection()));

    sendSocket = new QTcpSocket();
    connect(sendSocket,SIGNAL(connected()),this,SLOT(sendFileInfo()));
    connect(sendSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(continueToSend(qint64)));


}

MainWindow::~MainWindow()
{
    sendJson(Logout,ui->edtName->text()); // 销毁对象前先退出
    delete ui;
}

void MainWindow::showMessage(MessageType type, QString hint, QString content)
{
    QDateTime now = QDateTime::currentDateTime();
    if(type == Login || type == Logout || type == System)
    {
        ui->browserMessage->setTextColor(QColor(190,190,190));
        ui->browserMessage->append(hint + now.toString("  hh:mm:ss"));
        ui->browserMessage->append(content);
    }
    else if(type == Chat)
    {
        ui->browserMessage->setTextColor(QColor(70,130,180));
        ui->browserMessage->append(hint + now.toString("  hh:mm:ss"));
        ui->browserMessage->setTextColor(QColor(0,0,0));
        ui->browserMessage->append(content);
    }
}

bool MainWindow::localUserStatus()
{
    if(ui->edtName->isEnabled()) // 根据昵称输入框判断本地用户在线情况
        return false;
    return true;
}

void MainWindow::setLocalUserStatus(bool status)
{
    /* --- 根据用户状态设置按钮可用性 ---*/
    ui->edtName->setEnabled(!status);
    ui->edtMessage->setEnabled(status);
    ui->btnSendMessage->setEnabled(status);
    ui->btnLogin->setEnabled(!status);
    ui->btnLogout->setEnabled(status);
    ui->boxMask->setEnabled(!status);
}

void MainWindow::setLocalFileStatus(bool status)
{
    /* --- 根据文件传输可用性设置按钮可用性以及监听状态 ---*/
    ui->edtFinalIP->setEnabled(status);
    ui->edtFinalPort->setEnabled(status);
    ui->btnChooseFile->setEnabled(status);
    ui->btnListen->setEnabled(status);
    listenType = Unlisten;
    ui->btnListen->setText(tr("Listen"));
}

void MainWindow::sendJson(MessageType type, QString nick_name, QString content)
{
    QJsonObject obj;

    if(nick_name.isEmpty())
        return;

    if(type == Chat)
        obj.insert("type","chat");
    else if(type == Login)
        obj.insert("type","login");
    else if(type == Logout)
        obj.insert("type","logout");
    else if(type == Online)
        obj.insert("type","online");


    if(content != "") // content 默认为空
        obj.insert("content",content);

    obj.insert("nick-name",nick_name);

    QJsonDocument doc; // json 格式封装将要发送到聊天室内的信息
    doc.setObject(obj);

    QByteArray data = doc.toJson();

    messageSender->writeDatagram(data.data(),data.size(),QHostAddress(ui->boxMask->currentText()),DEFAULT_MESSAGE_PORT);
}

void MainWindow::readAllMessage()
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
                    showMessage(Chat,info,obj.take("content").toString());
                }
                else if(type.toString() == "login")
                {
                    /* --- 查找同名、同IP的用户信息 不存在则添加--- */
                    QList<QListWidgetItem *> user = ui->listOnlineUser->findItems(info, Qt::MatchExactly | Qt::MatchCaseSensitive );
                    if(user.size() == 0)
                        ui->listOnlineUser->insertItem(ui->listOnlineUser->count()+1,info);
                    showMessage(Login,info,tr(" -- enter the chat room"));
                    if(localUserStatus()) // 本地用户在线则向聊天室发送 Online 信息，交换在线用户信息
                        sendJson(Online,ui->edtName->text());
                }
                else if(type.toString() == "logout")
                {
                    /* --- 查找退出用户信息并从列表删除 --- */
                    QList<QListWidgetItem *> user = ui->listOnlineUser->findItems(info, Qt::MatchExactly | Qt::MatchCaseSensitive );
                    for(auto it = user.begin();it!=user.end();it++)
                    {
                        ui->listOnlineUser->removeItemWidget((*it));
                        delete (*it);
                    }
                    showMessage(Logout,info,tr(" -- quit the chat room"));
                }
                else if(type.toString() == "online")
                {
                    /* --- 更新在线用户信息 --- */
                    QList<QListWidgetItem *> user = ui->listOnlineUser->findItems(info, Qt::MatchExactly | Qt::MatchCaseSensitive );
                    if(user.size() == 0)
                        ui->listOnlineUser->insertItem(ui->listOnlineUser->count()+1,info);
                }
            }
        }
    }

}

void MainWindow::sendChatMessage()
{
    if(ui->edtMessage->toPlainText().isEmpty())
        QMessageBox::information(this,tr("No message"),tr("No message"),QMessageBox::Yes);
    else
    {
        sendJson(Chat,ui->edtName->text(),ui->edtMessage->toPlainText());
        ui->edtMessage->clear();
    }
}

void MainWindow::sendLoginMessage()
{
    if(!Tools::vaildNickName(ui->edtName->text()))
        QMessageBox::information(this,tr("Invaild Nickname!"),tr("Invaild Nickname!"),QMessageBox::Yes);
    else
    {
        setLocalUserStatus(true);
        setLocalFileStatus(true);
        ui->btnListen->setEnabled(true);
        sendJson(Login,ui->edtName->text());
    }
}

void MainWindow::sendLogoutMessage()
{
    setLocalUserStatus(false);
    setLocalFileStatus(false);
    sendJson(Logout,ui->edtName->text());
}

void MainWindow::chooseSendFile()
{
    QString name = QFileDialog::getOpenFileName(this, tr("Choose File"), ".", tr("All File(*.*)"));
    if(!name.isEmpty())
    {
        ui->btnSendFile->setEnabled(true);
        sendFile = new QFile(name);
        sendFile->open(QIODevice::ReadOnly);
        sendFileName = name.right(name.size()-name.lastIndexOf('/')-1);
        showMessage(System,tr("System"),tr(" -- File Selete: %1").arg(sendFileName));
        sendTimes = 0;
        sendFileBlock.clear();
    }
}

void MainWindow::listen()
{
    if(listenType == Unlisten)
    {
        ui->btnListen->setText(tr("UnListen"));
        listenType = Listen;
        fileServer->listen(QHostAddress(ui->edtFinalIP->text()),ui->edtFinalPort->text().toInt());
        showMessage(System,tr("System"),tr(" -- File Port Listening"));
    }
    else if(listenType == Listen)
    {
        ui->btnListen->setText(tr("Listen"));
        listenType = Unlisten;
        fileServer->close();
        showMessage(System,tr("System"),tr(" -- File Port Listening Closed"));
    }
}

void MainWindow::acceptConnection()
{
    receiveFileTotalSize = receiveFileTransSize = 0;
    showMessage(System,tr("System"),tr(" -- New File Arriving"));

    receiveSocket = fileServer->nextPendingConnection();
    connect(receiveSocket,SIGNAL(readyRead()),this,SLOT(readConnection()));

    ui->ProgressBar->show(); // 显示文件传输进度条
}

void MainWindow::readConnection()
{
    if(receiveFileTotalSize == 0)
    {
        QDataStream in(receiveSocket);
        in>>receiveFileTotalSize>>receiveFileTransSize>>receiveFileName;

        ui->ProgressBar->setMaximum(receiveFileTotalSize);

        QString name = receiveFileName.mid(0,receiveFileName.lastIndexOf("."));
        QString suffix = receiveFileName.mid(receiveFileName.lastIndexOf(".")+1,receiveFileName.size());

        qDebug()<<receiveFileName<<name<<suffix;
        if( QSysInfo::kernelType() == "linux" )
        {
            if(QFile::exists(receiveFileName))
            {
                int id = 1;
                while( QFile::exists( name + "(" + QString::number(id) + ")." + suffix ) )
                    id++;
                receiveFile = new QFile( name + "(" + QString::number(id) + ")." + suffix );
            }
            else
                receiveFile = new QFile(receiveFileName);
        }
        else
        {
            if(QFile::exists(DEFAULT_FILE_STORE+receiveFileName))
            {
                int id = 1;
                while( QFile::exists(DEFAULT_FILE_STORE + name + "(" + QString::number(id) + ")." + suffix) )
                    id++;
                receiveFile = new QFile(DEFAULT_FILE_STORE + name + "(" + QString::number(id) + ")." + suffix);
            }
            else
            {
                receiveFile = new QFile(DEFAULT_FILE_STORE+receiveFileName); // 保存文件
            }
        }

        receiveFile->open(QFile::ReadWrite);

        showMessage(System,tr("System"),tr(" -- File Name: %1 File Size: %2").arg(receiveFileName,QString::number(receiveFileTotalSize)));
    }
    else
    {
        receiveFileBlock = receiveSocket->readAll();
        receiveFileTransSize += receiveFileBlock.size();

        ui->ProgressBar->setValue(receiveFileTransSize); // 更新进度条

        receiveFile->write(receiveFileBlock);
        receiveFile->flush();
    }

    if(receiveFileTransSize == receiveFileTotalSize) // 文件传输完成
    {
        showMessage(System,tr("System"),tr(" -- File Transmission Complete"));

        QMessageBox::StandardButton choice;
        choice = QMessageBox::information(this,tr("Open File Folder?"),tr("Open File Folder?"),QMessageBox::Yes,QMessageBox::No);
        if(choice == QMessageBox::Yes)
        {
            if(QSysInfo::kernelType() == "linux")
            {
                QDir dir("");
                QDesktopServices::openUrl(QUrl(dir.absolutePath() , QUrl::TolerantMode)); // 打开文件夹
            }
            else
            {
                QDir dir(DEFAULT_FILE_STORE);
                QDesktopServices::openUrl(QUrl(dir.absolutePath() , QUrl::TolerantMode)); // 打开文件夹
            }
        }
        receiveFileTotalSize = receiveFileTransSize = 0;
        receiveFileName = QString::null;
        ui->ProgressBar->hide();
        receiveFile->close();
    }
}

void MainWindow::sendConnection()
{
    if(sendTimes == 0)
    {
        sendSocket->connectToHost(QHostAddress(ui->edtFinalIP->text()),ui->edtFinalPort->text().toInt());
        if(!sendSocket->waitForConnected(2000)) // 检测网络情况
            QMessageBox::information(this,tr("ERROR"),tr("Network Error"),QMessageBox::Yes);
        else
            sendTimes = 1;
    }
    else
        sendFileInfo();

}

void MainWindow::sendFileInfo()
{
    sendFileEachTransSize = 40 * 1024;

    QDataStream out(&sendFileBlock,QIODevice::WriteOnly);
    out<<qint64(0)<<qint64(0)<<sendFileName;

    sendFileTotalSize = sendFile->size() + sendFileBlock.size();
    sendFileLeftSize = sendFile->size() + sendFileBlock.size();

    out.device()->seek(0);
    out<<sendFileTotalSize<<qint64(sendFileBlock.size());

    sendSocket->write(sendFileBlock);

    ui->ProgressBar->setMaximum(sendFileTotalSize);
    ui->ProgressBar->setValue(0); // 更新进度条数值
    ui->ProgressBar->show(); // 显示进度条

    showMessage(System,tr("System"),tr(" -- File Name: %1 File Size: %2").arg(sendFileName,QString::number(sendFileTotalSize)));
}

void MainWindow::continueToSend(qint64 size)
{
    if(sendSocket->state() != QAbstractSocket::ConnectedState) // 网络出错
    {
        QMessageBox::information(this,tr("ERROR"),tr("Network Error"),QMessageBox::Yes);
        ui->ProgressBar->hide();
        return;
    }

    sendFileLeftSize -= size;

    if(sendFileLeftSize == 0) // 文件发送完成
    {
        showMessage(System,tr("System"),tr(" -- File Transmission Complete"));

        sendSocket->disconnectFromHost();

        ui->ProgressBar->hide();
    }
    else
    {
        sendFileBlock = sendFile->read( qMin(sendFileLeftSize,sendFileEachTransSize) );

        sendSocket->write(sendFileBlock);

        ui->ProgressBar->setValue(sendFileTotalSize - sendFileLeftSize);
    }
}
