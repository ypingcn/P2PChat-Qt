#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss(":/theme/default.qss"); // 加载主题
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();

    QSettings settings;
    if(!settings.contains("language"))
        settings.setValue("language","zh-cn");

    QTranslator Translator; // 加载翻译文件
    Translator.load("./local/"+settings.value("language").toString()+".qm");
    a.installTranslator(&Translator);

    MainWindow w;
    w.show();

    return a.exec();
}
