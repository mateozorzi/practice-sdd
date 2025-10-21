#include "coordinates.h"

#include <QVector>
#include <QPoint>

Coordinates::Coordinates(){}


void Coordinates::add_checkpoint(const QPoint& point){
    checkpoints.append(point);
}

bool Coordinates::is_a_checkpoint(const QPoint& point){
    //buscar si el punto es un checkpoint cargado
    return true;
}

void Coordinates::delete_checkpoint(const QPoint& point){
    //eliminar punto
}

Coordinates::~Coordinates(){}
