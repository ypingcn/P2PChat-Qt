#include "hintwidget.h"

#include <QTimer>
#include <QDebug>

hintWidget::hintWidget(QWidget *parent) : QWidget(parent)
{
    closeTimer = new QTimer(this);
    connect(closeTimer,SIGNAL(timeout()),this,SLOT(close()));

    setFixedHeight(32);

    setAutoFillBackground(true);
    setObjectName("hintWidget");

    labText = new QLabel();
    labText->setObjectName("hintContent");
    labText->setAlignment(Qt::AlignCenter);
    labText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    mainLayout = new QHBoxLayout();
    mainLayout->addWidget(labText);
//    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(3,3,5,3);
    setLayout(mainLayout);
}

void hintWidget::setText(QString content)
{
    labText->setText(content);

    if( !this->isVisible() )
    {
        this->show();
        closeTimer->start(1500);
    }
    else
    {
        closeTimer->stop();
        closeTimer->start(1500);
    }
}
