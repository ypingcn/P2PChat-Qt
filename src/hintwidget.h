#ifndef ERRORWIDGET_H
#define ERRORWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

class hintWidget : public QWidget
{
    Q_OBJECT
public:
    explicit hintWidget(QWidget *parent = 0);
    void setText(QString content);

private:
    QLabel * labText;
    QHBoxLayout * mainLayout;
    QTimer * closeTimer;
signals:

public slots:
};

#endif // ERRORWIDGET_H
