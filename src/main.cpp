#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss(":/theme/default.qss"); // 加载主题
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();

    QTranslator Translator; // 加载翻译文件
    Translator.load("zh-cn.qm");
    a.installTranslator(&Translator);

    MainWindow w;
    w.show();

    return a.exec();
}
