#include "checkpoint.h"

Checkpoint::Checkpoint(const int id, const char o, const QPoint& pos, const QRect& figure)
    : id(id)
    ,orientation(o)
    , click(pos)
    , rect(figure)
{}

Checkpoint::Checkpoint()
    : id(0)
    , orientation('\0')
    , click (QPoint(-1,-1))
    , rect (QRect())
{}

bool Checkpoint::operator==(const Checkpoint& other) const {
    return (orientation == other.orientation && click == other.click);
}

bool Checkpoint::contains(const QPoint& pos) const {
    return rect.contains(pos.x(), pos.y());
}

void Checkpoint::update_id(){
    id--;
}

