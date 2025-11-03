#include "race.h"

#include "coordinates.h"

Race::Race(const int id, const QString& name, const QString& city,const QString& map,
           const QString& mask, const int n_checkpoints, QVector<Checkpoint>& checkpoints):
    id(id), name(name), city(city), map(map), mask(mask),
    n_checkpoints(n_checkpoints), checkpoints(Coordinates(checkpoints)) {}
