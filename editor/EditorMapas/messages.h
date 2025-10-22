#ifndef MESSAGES_H
#define MESSAGES_H

#include <QString>
#include <QPoint>
#include <QWidget>

class Messages
{
public:
    Messages();
    void checkpoint_message(QWidget* parent, QString title, QString body, QPoint point);
};

#endif // MESSAGES_H
