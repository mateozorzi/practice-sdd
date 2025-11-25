#include "messages.h"

#include <QMessageBox>
#include <QPoint>
#include <QString>
#include <QWidget>


Messages::Messages() {}

void Messages::checkpoint_message(QWidget* parent, const QString& title, const QString& body,
                                  const QPoint& point) {
    QMessageBox::information(parent, title, body.arg(point.x()).arg(point.y()));
}

void Messages::message_no_args(QWidget* parent, const QString& title, const QString& body) {
    QMessageBox::information(parent, title, body);
}
