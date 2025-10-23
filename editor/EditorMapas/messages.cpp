#include "messages.h"
#include <QMessageBox>
#include <QString>
#include <QPoint>
#include <QWidget>



Messages::Messages() {}

void Messages::checkpoint_message(QWidget* parent, QString title, QString body, QPoint point){
    QMessageBox::information(parent, title,
                             body.arg(point.x()).arg(point.y()));
}

void Messages::message_no_args(QWidget *parent, QString title, QString body){
    QMessageBox::information(parent, title,body);
}
