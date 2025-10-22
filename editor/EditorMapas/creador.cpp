#include "creador.h"
#include "ui_creador.h"
#include "coordinates.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QPoint>
#include <QPen>
#include <QBrush>
#include <QList>
#include <cmath>


Creador::Creador(const QString& path, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Creador)
    , map(path)
    , checkpoints(Coordinates())
    , mode(ADD)
{
    ui->setupUi(this);
    set_background(map);

    ui->labelmap->installEventFilter(this);


    ui->addButton->setCheckable(true);
    ui->removeButton->setCheckable(true);
    connect(ui->addButton, &QPushButton::toggled, this, &Creador::on_addButton_toggled);
    connect(ui->removeButton, &QPushButton::toggled, this, &Creador::on_removeButton_toggled);

    ui->addButton->setChecked(true);
}

void Creador::set_background(const QString& path){
    QPixmap map(path);
    if(!map.isNull()) {
        // Escalar la imagen al tamaño del label
        ui->labelmap->setPixmap(map.scaled(
            ui->labelmap->size(),
            Qt::KeepAspectRatio,  // mantiene proporción
            Qt::SmoothTransformation
            ));
    }
}

//void Creador::update_map(const QPoint& pos){
//
//    if (mode == ADD){
//        // Agregar el checkpoint centrado en pos
//        int radio = 2;
//        scene->addEllipse(pos.x()-radio, pos.y()-radio,
//                          2*radio, 2*radio, QPen(Qt::red), QBrush(Qt::yellow));
//    }else if (mode == REMOVE){
//        // Recorrer todos los items de la escena
//        QList<QGraphicsItem*> items = scene->items();
//        for (QGraphicsItem* item : items) {
//            if (auto ellipse = dynamic_cast<QGraphicsEllipseItem*>(item)) {
//                QRectF rect = ellipse->rect();
//                QPointF center = ellipse->pos() + QPointF(rect.width()/2, rect.height()/2);
//
//                // Comparamos exacto con pos
//                if (center == pos) {
//                    scene->removeItem(item);
//                    delete item;
//                    break; // eliminamos solo uno
//                }
//            }
//        }
//    }
//}

void Creador::add_checkpoint(QEvent *event){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    QPoint pos = mouseEvent->pos();

    if(checkpoints.add_checkpoint(pos)){
        QMessageBox::information(this, "Success",
                                 QString("Checkpoint added in (%1, %2)")
                                     .arg(pos.x()).arg(pos.y()));

        //update_map()
    }else{
        QMessageBox::information(this, "Error",
                                 QString("Can´t add checkpoint in (%1, %2)")
                                     .arg(pos.x()).arg(pos.y()));
    }
}

void Creador::delete_checkpoint(QEvent *event){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    // Posición dentro del label
    QPoint pos = mouseEvent->pos();

    QPoint del = checkpoints.delete_checkpoint(pos);

    // Guardar coordenada
    if(del.x() != -1){
        QMessageBox::information(this, "Success",
                                 QString("Checkpoint delete in (%1, %2)")
                                     .arg(pos.x()).arg(pos.y()));

        //update_map()
    }else{
        QMessageBox::information(this, "Error",
                                 QString("No checkpoint in (%1, %2)")
                                     .arg(pos.x()).arg(pos.y()));
    }
}

void Creador::handle_checkpoints(QEvent *event){
    //case, add button activate
    if (mode == ADD){
        add_checkpoint(event);
    }else if(mode == REMOVE){
        delete_checkpoint(event);
    }

}

bool Creador::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->labelmap && event->type() == QEvent::MouseButtonPress){
        this->handle_checkpoints(event);
        return true; // evento manejado
    }
    return QMainWindow::eventFilter(obj, event);
}




Creador::~Creador()
{
    delete ui;
}


void Creador::on_Exit_clicked()
{
    emit close_w();
    this->close();
}

void Creador::on_addButton_toggled(bool checked)
{
    if (checked) {
        ui->removeButton->setChecked(false);
        mode = ADD;
    } else {
        mode = NONE;
    }
}


void Creador::on_removeButton_toggled(bool checked)
{
    if (checked) {
        ui->addButton->setChecked(false);
        mode = REMOVE;
    } else {
        mode = NONE;
    }
}

