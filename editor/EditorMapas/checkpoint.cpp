#include "checkpoint.h"

Checkpoint::Checkpoint(const int id, const char o, const QPoint& pos, const QRect& figure)
    : id(id)
    ,orientation(o)
    , click(pos)
    , rect(figure)
{}

bool Checkpoint::operator==(const Checkpoint& other) const {
    return (orientation == other.get_orientation() && click == other.get_click());
}

bool Checkpoint::contains(const QPoint& pos) const {
    return rect.contains(pos.x(), pos.y());
}

int Checkpoint::get_id() const{
    return id;
}

void Checkpoint::update_id(){
    id--;
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
