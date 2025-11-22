#ifndef RACE_H
#define RACE_H

#include <QString>
#include <QVector>

#include "checkpoint.h"
#include "coordinates.h"

struct Race {
public:
    int id;
    QString name;
    QString city;
    QString map;
    QString mask;
    int n_checkpoints;
    Coordinates checkpoints;
    int n_laps;

    Race(const int id, const QString& name, const QString& city, const QString& map,
         const QString& mask, const int n_checkpoints, QVector<Checkpoint>& checkpoints,
         const int n_laps);
    Race();
};

#endif  // RACE_H
