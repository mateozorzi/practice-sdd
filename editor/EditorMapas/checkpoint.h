#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <QPoint>
#include <QRect>
#include <QString>

struct Checkpoint {
public:
    int id;
    char orientation;
    QPoint click;
    QRect rect;

    Checkpoint(const int id, const char o, const QPoint& pos, const QRect& figure);
    Checkpoint();
    bool contains(const QPoint& pos) const;
    void update_id();
    bool operator==(const Checkpoint& other) const;
};

#endif  // CHECKPOINT_H
