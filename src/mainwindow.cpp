#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QMessageBox>
#include <QRegExp>
#include <QNetworkInterface>
#include <QFileDialog>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->ProgressBar->hide();

    ui->labIPAdress->setText(getLocalIP());
    ui->edtFinalIP->setText("127.0.0.2");
    ui->edtFinalPort->setText(QString::number(FILE_PORT));

    ui->btnSendFile->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->edtMessage->setEnabled(false);
    ui->btnSendMessage->setEnabled(false);
    ui->btnStop->hide();

    state = NoState;
    sendTimes = 0;
    QFont font;
    font.setPixelSize(14);
    ui->browserMessage->setFont(font);

    messageSender = new QUdpSocket();
    messageReader = new QUdpSocket();

    messageReader->bind(DEFAULT_MESSAGE_PORT,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint );
    connect(messageReader,SIGNAL(readyRead()),this,SLOT(readAllMessage()));

    connect(ui->btnLogin,SIGNAL(clicked()),this,SLOT(sendLoginMessage()));
    connect(ui->btnLogout,SIGNAL(clicked(bool)),this,SLOT(sendLogoutMessage()));
    connect(ui->btnSendMessage,SIGNAL(clicked()),this,SLOT(sendChatMessage()));
    connect(ui->btnChooseFile,SIGNAL(clicked(bool)),this,SLOT(chooseSendFile()));

    fileServer = new QTcpServer();
    connect(fileServer,SIGNAL(newConnection()),this,SLOT(acceptConnection()));

    sendSocket = new QTcpSocket();
    connect(sendSocket,SIGNAL(connected()),this,SLOT(sendFileInfo()));

    connect(ui->btnListen,SIGNAL(clicked(bool)),this,SLOT(listen()));
    connect(ui->btnSendFile,SIGNAL(clicked(bool)),this,SLOT(sendConnection()));
}

MainWindow::~MainWindow()
{
    sendJson(Logout,ui->edtName->text());
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

QString MainWindow::toIPv4(quint32 arg)
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

QString MainWindow::getLocalIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach(QHostAddress in,list)
    {
        if(in.protocol() == QAbstractSocket::IPv4Protocol && in.toString().contains("192.168"))
            return in.toString();
    }
    return QString::null;
}

bool MainWindow::vaildNickName(QString name)
{
    QRegExp reg1("[A-Za-z0-9_]{1,}");
    QRegExp reg2("[\u4E00-\u9FA5]{1,}");
    if(reg1.exactMatch(name))
        return true;
    if(reg2.exactMatch(name))
        return true;
    return false;
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


    if(content != "")
        obj.insert("content",content);

    obj.insert("nick-name",nick_name);

    QJsonDocument doc;
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
        QHostAddress source;
        messageReader->readDatagram(data.data(),data.size(),&source);

        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(data,&jsonError);
        if(jsonError.error == QJsonParseError::NoError && doc.isObject())
        {
            QJsonObject obj = doc.object();
            if(obj.contains("type") && obj.contains("nick-name"))
            {
                QJsonValue type = obj.take("type");
                QString info = obj.take("nick-name").toString() + "(" + toIPv4(source.toIPv4Address()) + ")" ;
                if(type.toString() == "chat" && obj.contains("content"))
                {
                    showMessage(Chat,info,obj.take("content").toString());
                }
                else if(type.toString() == "login")
                {
                    QList<QListWidgetItem *> user = ui->listOnlineUser->findItems(info, Qt::MatchExactly | Qt::MatchCaseSensitive );
                    if(user.size() == 0)
                        ui->listOnlineUser->insertItem(ui->listOnlineUser->count()+1,info);
                    showMessage(Login,info,tr(" -- enter the chat room"));
                    if(!ui->edtName->isEnabled())
                        sendJson(Online,ui->edtName->text());
                }
                else if(type.toString() == "logout")
                {
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
    if(!ui->edtName->isEnabled())
        QMessageBox::information(this,tr("Have logined!"),tr("Have logined!"),QMessageBox::Yes);
    else if(!vaildNickName(ui->edtName->text()))
        QMessageBox::information(this,tr("Invaild Nickname!"),tr("Invaild Nickname!"),QMessageBox::Yes);
    else
    {
        ui->edtName->setEnabled(false);
        ui->edtMessage->setEnabled(true);
        ui->btnSendMessage->setEnabled(true);
        sendJson(Login,ui->edtName->text());
    }
}

void MainWindow::sendLogoutMessage()
{
    if(ui->edtName->isEnabled())
        QMessageBox::information(this,tr("Have not logined!"),tr("Have not logined!"),QMessageBox::Yes);
    else
    {
        ui->edtName->setEnabled(true);
        ui->edtMessage->setEnabled(false);
        ui->btnSendMessage->setEnabled(false);
        sendJson(Logout,ui->edtName->text());
    }

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
        sendFileTotalSize = sendFileLeftSize = 0;
        sendFileBlock.clear();
    }
}

void MainWindow::listen()
{
    if(ui->edtName->isEnabled())
        QMessageBox::information(this,tr("Have logined!"),tr("Login to continue"),QMessageBox::Yes);
    else if(state == NoState)
    {
        ui->edtFinalIP->setEnabled(false);
        ui->edtFinalPort->setEnabled(false);
        ui->btnSendFile->setEnabled(false);
        ui->btnListen->setText(tr("UnListen"));
        fileServer->listen(QHostAddress(ui->edtFinalIP->text()),ui->edtFinalPort->text().toInt());
        showMessage(System,tr("System"),tr(" -- File Port Listening"));
        state = ReceiveFile;
    }
    else if(state == ReceiveFile)
    {
        ui->edtFinalIP->setEnabled(true);
        ui->edtFinalPort->setEnabled(true);
        ui->btnListen->setText(tr("Listen"));
        fileServer->close();
        showMessage(System,tr("System"),tr(" -- File Port Listening Closed"));
        state = NoState;
    }
}

void MainWindow::acceptConnection()
{
    receiveFileTotalSize = receiveFileTransSize = 0;
    showMessage(System,tr("System"),tr(" -- New File Arriving"));

    receiveSocket = fileServer->nextPendingConnection();
    connect(receiveSocket,SIGNAL(readyRead()),this,SLOT(readConnection()));

    ui->ProgressBar->show();
    ui->btnStop->setEnabled(true);
    connect(ui->btnStop,SIGNAL(clicked(bool)),this,SLOT(stopToRecvFile()));
}

void MainWindow::stopToRecvFile()
{
    receiveSocket->close();
}

void MainWindow::readConnection()
{
    if(receiveFileTotalSize == 0)
    {
        QDataStream in(receiveSocket);
        in>>receiveFileTotalSize>>receiveFileTransSize>>receiveFileName;

        ui->ProgressBar->setMaximum(receiveFileTotalSize);

        receiveFile = new QFile("D:\\"+receiveFileName);
        receiveFile->open(QFile::ReadWrite);

        showMessage(System,tr("System"),tr(" -- File Name: %1 File Size: %2").arg(receiveFileName,QString::number(receiveFileTotalSize)));
    }
    else
    {
        receiveFileBlock = receiveSocket->readAll();
        receiveFileTransSize += receiveFileBlock.size();

        ui->ProgressBar->setValue(receiveFileTransSize);

        receiveFile->write(receiveFileBlock);
        receiveFile->flush();
    }

    if(receiveFileTransSize == receiveFileTotalSize)
    {
        showMessage(System,tr("System"),tr(" -- File Transmission Complete"));

        QMessageBox::StandardButton choice;
        choice = QMessageBox::information(this,tr("Open File Folder?"),tr("Open File Folder?"),QMessageBox::Yes,QMessageBox::No);
        if(choice == QMessageBox::Yes)
        {
            QDir dir("D://");
            QDesktopServices::openUrl(QUrl(dir.absolutePath() , QUrl::TolerantMode));
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
        if(!sendSocket->waitForConnected(100))
            QMessageBox::information(this,tr("ERROR"),tr("network error"),QMessageBox::Yes);
        else
            sendTimes = 1;
    }
    else
        sendFileInfo();

}

void MainWindow::sendFileInfo()
{
    sendFileEachTransSize = 4 * 1024;

    QDataStream out(&sendFileBlock,QIODevice::WriteOnly);
    out<<qint64(0)<<qint64(0)<<sendFileName;

    sendFileTotalSize += sendFile->size() + sendFileBlock.size();
    sendFileLeftSize += sendFile->size() + sendFileBlock.size();

    out.device()->seek(0);
    out<<sendFileTotalSize<<qint64(sendFileBlock.size());

    sendSocket->write(sendFileBlock);

    ui->ProgressBar->setMaximum(sendFileTotalSize);
    ui->ProgressBar->setValue(sendFileTotalSize - sendFileLeftSize);
    ui->ProgressBar->show();

    showMessage(System,tr("System"),tr(" -- File Name: %1 File Size: %2").arg(sendFileName,QString::number(sendFileTotalSize)));

    connect(sendSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(continueToSend(qint64)));
}

void MainWindow::continueToSend(qint64 size)
{
    if(sendSocket->state() != QAbstractSocket::ConnectedState)
    {
        QMessageBox::information(this,tr("error"),tr("error"),QMessageBox::Yes);
        ui->ProgressBar->hide();
        return;
    }

    sendFileLeftSize -= size;

    sendFileBlock = sendFile->read( qMin(sendFileLeftSize,sendFileEachTransSize) );

    sendSocket->write(sendFileBlock);

    ui->ProgressBar->setValue(sendFileTotalSize - sendFileLeftSize);

    if(sendFileLeftSize == 0)
    {
        showMessage(System,tr("System"),tr(" -- File Transmission Complete"));

        ui->ProgressBar->hide();

        sendSocket->disconnectFromHost();
        sendSocket->close();
        sendTimes = 0;
        sendFileLeftSize  = sendFileTotalSize ;

    }
}


