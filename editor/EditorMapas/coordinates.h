#ifndef COORDINATES_H
#define COORDINATES_H

#include <QVector>
#include <QPoint>
#include <QObject>
#include "checkpoint.h"

class Coordinates{
private:
    QVector<Checkpoint> checkpoints;

    bool point_too_close(const Checkpoint& point);

    bool is_a_checkpoint(const Checkpoint& point);

    QString get_map_name(const QString& map);


public:
    Coordinates();
    Coordinates(QVector<Checkpoint>& checkpoint);
    ~Coordinates();

    bool add_checkpoint(const Checkpoint& point);

    Checkpoint delete_checkpoint(const QPoint& point);

    void update_id(int id_delete);

    bool enough_checkpoints();

    bool export_checkpoints(const QString& name, const QString& map, const int n_laps, const QString& mask, QWidget* window);

    QVector<Checkpoint> get_checkpoints();

    bool is_empty();

    Checkpoint last_checkpoint();
};

#endif // COORDINATES_H
