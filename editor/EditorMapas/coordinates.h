#ifndef COORDINATES_H
#define COORDINATES_H

#include <QVector>
#include <QPoint>

class Coordinates
{

private:
    QVector<QPoint> checkpoints;

    bool point_too_close(const QPoint& point);

    bool is_a_checkpoint(const QPoint& point);


public:
    Coordinates();
    ~Coordinates();

    bool add_checkpoint(const QPoint& point);

    QPoint delete_checkpoint(const QPoint& point);
};

#endif // COORDINATES_H
