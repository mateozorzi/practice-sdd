#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <QString>
#include <QPoint>
#include <QRect>

class Checkpoint
{
private:
    char orientation;
    QPoint click;
    QRect rect;


public:
    Checkpoint(const char o, const QPoint& pos, const QRect& figure);
    bool contains(const QPoint& pos) const;
    QPoint get_click() const;
    QRect get_rect() const;
    char get_orientation() const;
    bool operator==(const Checkpoint& other) const;
};

#endif // CHECKPOINT_H
