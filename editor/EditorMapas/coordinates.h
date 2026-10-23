#ifndef COORDINATES_H
#define COORDINATES_H

#include <QObject>
#include <QPoint>
#include <QVector>

#include "checkpoint.h"

class Coordinates {
private:
    QVector<Checkpoint> checkpoints;

    bool point_too_close(const Checkpoint& point);

    bool is_a_checkpoint(const Checkpoint& point);

    static QString get_map_name(const QString& map);


public:
    Coordinates();
    explicit Coordinates(QVector<Checkpoint>& checkpoint);
    ~Coordinates();

    bool add_checkpoint(const Checkpoint& point);

    Checkpoint delete_checkpoint(const QPoint& point);

    void update_id(int id_delete);

    bool enough_checkpoints();

    bool export_checkpoints(const QString& races_folder, const QString& race_name,
                            const QString& map, const int n_laps, const QString& mask,
                            QWidget* window);

    const QVector<Checkpoint>& get_checkpoints() const;

    bool is_empty();

    Checkpoint last_checkpoint();
};

#endif  // COORDINATES_H
