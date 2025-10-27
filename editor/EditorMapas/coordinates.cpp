#include "coordinates.h"

#include <QVector>
#include <QPoint>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QString>
#include "Constants.h"

Coordinates::Coordinates(){}

bool Coordinates::point_too_close(const Checkpoint& checkpoint){
    QPoint pos = checkpoint.get_click();
    for (const Checkpoint& p: checkpoints){
        if(p.contains(pos)){
            return true;
        }
    }

    return false;
}

bool Coordinates::is_a_checkpoint(const Checkpoint& checkpoint){
    //looking for a checkpoint or
    //if is too close to another checkpoint
    if (checkpoints.contains(checkpoint)){
        return true;
    }

    if (point_too_close(checkpoint)){
        return true;
    }

    return false;
}

bool Coordinates::add_checkpoint(const Checkpoint& checkpoint){
    if (is_a_checkpoint(checkpoint)){
        //checkpoint already exists
        return false;
    }

    checkpoints.append(checkpoint);
    return true;
}


QPoint Coordinates::delete_checkpoint(const QPoint& point){
    QPoint del(-1, -1);

    for (auto it = checkpoints.begin(); it != checkpoints.end(); ++it) {
        if (it->contains(point)) {
            del = it->get_click();
            checkpoints.erase(it);
            break;
        }
    }

    return del;
}

bool Coordinates::enough_checkpoints(){
    return (checkpoints.size() >= MIN_CHECKPOINTS);
}

bool Coordinates::export_checkpoints(const QString& map){
    QString path = QCoreApplication::applicationDirPath() + "/../server/races";
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
    out << "id, orientation, xCenter, yCenter, height, width\n";
    int id_checkpoint = 0;
    for (const Checkpoint& p : checkpoints){
        char orientation = p.get_orientation();
        QRect r= p.get_rect();
        QPoint center = r.center();
        int height = r.height();
        int width = r.width();
        out << id_checkpoint <<","<<orientation<< "," << center.x() << "," <<
            center.y()<<","<< height<<","<<width<<"\n";
        id_checkpoint++;
    }

    archivo.close();

    return true;
}

bool Coordinates::is_empty(){
    return checkpoints.empty();
}

Checkpoint Coordinates::last_checkpoint(){
    return checkpoints.back();
}

Coordinates::~Coordinates(){}
