#include "coordinates.h"

#include <QVector>
#include <QPoint>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QString>

#define MIN_DIST 30
#define MIN_CHECKPOINTS 5

Coordinates::Coordinates(){}

bool Coordinates::point_too_close(const QPoint& point){
    //distance <= 30, too close
    int distance = 0;
    for (const QPoint& p: checkpoints){
        distance = 0;
        int d_x = 0;
        int d_y = 0;

        d_x = (point.x()-p.x());
        d_y = (point.y()-p.y());

        distance = std::sqrt(d_x * d_x + d_y * d_y);

        if (distance <= MIN_DIST){
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

        if (distance <= MIN_DIST) {
            del = *it;
            checkpoints.erase(it);
            break;  // delete
        }
    }

    return del;
}

bool Coordinates::enough_checkpoints(){
    return (checkpoints.size() >= MIN_CHECKPOINTS);
}

bool Coordinates::export_checkpoints(const QString& map){
    QString path = QCoreApplication::applicationDirPath() + "/../editor/races";
    QDir dir(path);

    if (!dir.exists() && !dir.mkpath(".")) {
        return false;
    }

    QFileInfo fileInfo(map);
    QString baseName = fileInfo.baseName();

    QString race_name = baseName + ".csv";
    QString path_race = dir.filePath(race_name);

    int id = 1;
    while (QFile::exists(path_race)) {
        race_name = baseName + "_" + QString::number(id) + ".csv";
        path_race = dir.filePath(race_name);
        id++;
    }
    QFile archivo(path_race);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&archivo);
    out << "x,y\n";
    int id_checkpoint = 0;
    for (const QPoint& point : checkpoints){
        out << id_checkpoint << "," << point.x() << "," << point.y() <<"\n";
        id_checkpoint++;
    }

    archivo.close();

    return true;
}

Coordinates::~Coordinates(){}
