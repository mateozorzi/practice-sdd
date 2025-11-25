#ifndef MESSAGES_H
#define MESSAGES_H

#include <QPoint>
#include <QString>
#include <QWidget>

class Messages {
public:
    Messages();
    void checkpoint_message(QWidget* parent, const QString& title, const QString& body,
                            const QPoint& point);
    void message_no_args(QWidget* parent, const QString& title, const QString& body);
};

#endif  // MESSAGES_H
