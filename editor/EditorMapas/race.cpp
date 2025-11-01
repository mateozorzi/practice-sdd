#include "race.h"

#include "coordinates.h"

Race::Race(const int id, const QString& name, const QString& city, const int n_checkpoints, QVector<Checkpoint>& checkpoints):
    id(id), name(name), city(city), n_checkpoints(n_checkpoints), checkpoints(Coordinates(checkpoints)) {}
