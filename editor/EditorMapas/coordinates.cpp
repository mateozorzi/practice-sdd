#include "coordinates.h"

#include <QVector>
#include <QPoint>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QString>
#include <QMessageBox>
#include <QWidget>
#include <QStringLiteral>
#include "Constants.h"

Coordinates::Coordinates(){}

Coordinates::Coordinates(QVector<Checkpoint>& checkpoint)
    : checkpoints(checkpoint)
{}

bool Coordinates::point_too_close(const Checkpoint& checkpoint){
    QPoint pos = checkpoint.click;
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

void Coordinates::update_id(int id_delete){
    for (int i = 0; i < checkpoints.size(); ++i) {
        if (checkpoints[i].id > id_delete){
            checkpoints[i].update_id();
        }
    }
}


Checkpoint Coordinates::delete_checkpoint(const QPoint& point){

    Checkpoint del = Checkpoint();

    for (auto it = checkpoints.begin(); it != checkpoints.end(); ++it) {
        if (it->contains(point)) {
            del = *it;
            checkpoints.erase(it);
            break;
        }
    }

    return del;
}

bool Coordinates::enough_checkpoints(){
    return (checkpoints.size() >= MIN_CHECKPOINTS);
}

QString Coordinates::get_map_name(const QString& map){
    QFileInfo fileInfo(map);
    QString filename = fileInfo.baseName();

    // Buscamos la última aparición de " - "
    int lastDash = filename.lastIndexOf(" - ");
    QString city = filename.mid(lastDash + 3);

    return city;
}

bool Coordinates::export_checkpoints(const QString& name, const QString& map, const int n_laps,const QString& mask, QWidget* window){
    QString path = QCoreApplication::applicationDirPath() + "/../server/races";
    QDir dir(path);

    if (!dir.exists() && !dir.mkpath(".")) {
        return false;
    }


    QString race_name = name + ".csv";
    QString path_race = dir.filePath(race_name);


    if (QFile::exists(path_race)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
        window,
        "Warning",
        "Already exists a race with this name. Do you want to overwrite it?",
        QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No){
            //new race name (+ "_i")
            int id = 1;
            while (QFile::exists(path_race)) {
                race_name = name + "_" + QString::number(id) + ".csv";
                path_race = dir.filePath(race_name);
                id++;
            }
        }
    }

    QFile archivo(path_race);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QString city = get_map_name(map);

    QTextStream out(&archivo);
    out << map << "," << mask << "\n";
    out << "City"<<","<<"n_laps"<<"\n";
    out << city <<","<<n_laps<<"\n";
    out << "id, orientation, xCenter, yCenter, height, width,"<<"\n";
    for (const Checkpoint& p : checkpoints){
        int id_checkpoint = p.id;
        char orientation = p.orientation;
        QRect r= p.rect;
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

QVector<Checkpoint> Coordinates::get_checkpoints(){
    return checkpoints;
}

bool Coordinates::is_empty(){
    return checkpoints.empty();
}

Checkpoint Coordinates::last_checkpoint(){
    return checkpoints.back();
}

Coordinates::~Coordinates(){}
