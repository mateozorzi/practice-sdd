#include "coordinates.h"

#include <QVector>
#include <QPoint>

#define DIST_MIN 30

Coordinates::Coordinates(){}

bool Coordinates::point_too_close(const QPoint& point){
    //distance <= 4, too close
    int distance = 0;
    for (const QPoint& p: checkpoints){
        distance = 0;
        int d_x = 0;
        int d_y = 0;

        d_x = (point.x()-p.x());
        d_y = (point.y()-p.y());

        distance = std::sqrt(d_x * d_x + d_y * d_y);

        if (distance <= DIST_MIN){
            return true;
        }
    }

    return false;
}

bool Coordinates::is_a_checkpoint(const QPoint& point){
    //looking for a checkpoint or
    //if is too close to another checkpoint
    if (checkpoints.contains(point)){
        return true;
    }

    if (point_too_close(point)){
        return true;
    }

    return false;
}

bool Coordinates::add_checkpoint(const QPoint& point){
    if (is_a_checkpoint(point)){
        //checkpoint already exists
        return false;
    }

    checkpoints.append(point);
    return true;
}


QPoint Coordinates::delete_checkpoint(const QPoint& point){
    QPoint del(-1,-1);
    //looking to delete a checkpoint
    for (auto it = checkpoints.begin(); it != checkpoints.end(); ++it) {
        int dx = point.x() - it->x();
        int dy = point.y() - it->y();
        double distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= DIST_MIN) {
            del = *it;
            checkpoints.erase(it);
            break;  // delete
        }
    }

    return del;
}

int Coordinates::total_checkpoints(){
    return (checkpoints.size());
}

Coordinates::~Coordinates(){}
