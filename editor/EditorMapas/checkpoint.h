#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <QString>
#include <QPoint>
#include <QRect>

struct Checkpoint
{
public:
    int id;
    char orientation;
    QPoint click;
    QRect rect;

    Checkpoint(const int id,const char o, const QPoint& pos, const QRect& figure);
    bool contains(const QPoint& pos) const;
    int get_id() const;
    void update_id();
    QPoint get_click() const;
    QRect get_rect() const;
    char get_orientation() const;
    bool operator==(const Checkpoint& other) const;
};

#endif // CHECKPOINT_H
