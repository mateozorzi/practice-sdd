#ifndef COORDINATES_H
#define COORDINATES_H

#include <QVector>
#include <QPoint>

class Coordinates
{

private:
    QVector<QPoint> checkpoints;



public:
    Coordinates();
    ~Coordinates();

    void add_checkpoint(const QPoint& point);

    bool is_a_checkpoint(const QPoint& point);

    void delete_checkpoint(const QPoint& point);
};

#endif // COORDINATES_H
