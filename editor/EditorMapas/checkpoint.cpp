#include "checkpoint.h"

Checkpoint::Checkpoint(const char o, const QPoint& pos, const QRect& figure)
    : orientation(o)
    , click(pos)
    , rect(figure)
{}

bool Checkpoint::operator==(const Checkpoint& other) const {
    return (orientation == other.get_orientation() && click == other.get_click());
}

bool Checkpoint::contains(const QPoint& pos) const {
    return rect.contains(pos.x(), pos.y());
}

QPoint Checkpoint::get_click() const {
    return click;
}

QRect Checkpoint::get_rect() const {
    return rect;
}

char Checkpoint::get_orientation() const{
    return orientation;
}
