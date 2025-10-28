#ifndef COORDINATES_H
#define COORDINATES_H

#include <QVector>
#include <QPoint>
#include "checkpoint.h"

class Coordinates
{

private:
    QVector<Checkpoint> checkpoints;

    bool point_too_close(const Checkpoint& point);

    bool is_a_checkpoint(const Checkpoint& point);


public:
    Coordinates();
    ~Coordinates();

    bool add_checkpoint(const Checkpoint& point);

    QPoint delete_checkpoint(const QPoint& point);

    bool enough_checkpoints();

    bool export_checkpoints(const QString& map);

    bool is_empty();

    Checkpoint last_checkpoint();
};

#endif // COORDINATES_H
