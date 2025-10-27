#include "creador.h"
#include "ui_creador.h"
#include "coordinates.h"
#include "messages.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QPoint>
#include <QPen>
#include <QBrush>
#include <QList>
#include <cmath>
#include <QDir>
#include <stdio.h>

#define RADIO 8
#define MIN_CHECKPOINTS 5


Creador::Creador(const QString& path, const QString& path_mask, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Creador)
    , map(path)
    , mask(path_mask)
    , checkpoints(Coordinates())
    , m(Messages())
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->labelmap->setScene(scene);

    set_background(map);

    mask_circuit.load(mask);

    ui->n_checkpoints->display(0);


    ui->labelmap->installEventFilter(this);

}

void Creador::set_background(const QString& path){
    QPixmap mapPixmap(path);
    if(!mapPixmap.isNull()) {
        scene->clear();
        ui->labelmap->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        scene->addPixmap(mapPixmap);
        scene->setSceneRect(mapPixmap.rect());
    }
}

void Creador::update_counter() {
    int count = 0;
    for (QGraphicsItem* item : scene->items()) {
        if (dynamic_cast<QGraphicsEllipseItem*>(item)) {
            count++;
        }
    }
    ui->n_checkpoints->display(count);
}


void Creador::update_map(const QPoint& pos, int mode){
    if (mode == ADD){
        QGraphicsEllipseItem* ellipse = scene->addEllipse(-RADIO, -RADIO, 2*RADIO, 2*RADIO,
                                                          QPen(Qt::red), QBrush(Qt::yellow));
        ellipse->setPos(pos);

    }else if (mode == REMOVE){
        QPointF clickPos = pos;
        for (QGraphicsItem* item : scene->items()) {
            if (auto ellipse = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                QPointF center = ellipse->pos();
                qreal dist = QLineF(clickPos, center).length();
                if (dist <= RADIO) {
                    scene->removeItem(item);
                    delete item;
                    break;
                }
            }
        }
    }

    update_counter();
}

bool Creador::is_street(const QPoint& pos){
    QColor streetColor(255, 36, 250); //de liberty

    QColor pixelColor = mask_circuit.pixelColor(pos);

    qDebug() << "mask: "<< streetColor << ", pixel: "<<pixelColor;

    int tolerance = 30; // tolerancia al color (opcional)
    bool isStreet =
        std::abs(pixelColor.red() - streetColor.red()) < tolerance &&
        std::abs(pixelColor.green() - streetColor.green()) < tolerance &&
        std::abs(pixelColor.blue() - streetColor.blue()) < tolerance;

    if (isStreet){
        //street
        return true;
    }
    return false;
}

void Creador::add_checkpoint(QMouseEvent *event){

    QPointF scenePos = ui->labelmap->mapToScene(event->pos());
    QPoint pos = scenePos.toPoint();

    if(is_street(pos) && checkpoints.add_checkpoint(pos)){
        update_map(pos, ADD);
    }else{
        m.checkpoint_message(this, "Error", "Can´t add checkpoint in (%1, %2)", pos);
    }
}

void Creador::delete_checkpoint(QMouseEvent *event){

    QPointF scenePos = ui->labelmap->mapToScene(event->pos());
    QPoint pos = scenePos.toPoint();

    QPoint del = checkpoints.delete_checkpoint(pos);

    if(del.x() != -1){
        update_map(del, REMOVE);
    }else{
        m.checkpoint_message(this, "Error", "No checkpoint in (%1, %2)", pos);
    }
}

void Creador::handle_checkpoints(QMouseEvent *event){
    //handle leftbutton or right button
    if (event->button() == Qt::LeftButton){
        add_checkpoint(event);
    }else if(event->button() == Qt::RightButton){
        delete_checkpoint(event);
    }

}

bool Creador::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->labelmap && event->type() == QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        this->handle_checkpoints(mouseEvent);
        return true; // evento manejado
    }
    return QMainWindow::eventFilter(obj, event);
}




Creador::~Creador()
{
    qDeleteAll(scene->items());
    delete scene;
    delete ui;
}


void Creador::on_Exit_clicked()
{
    emit close_w();
    this->close();
}

void Creador::on_Done_clicked()
{
    //create csv with the checkpoints of the race

    if (!checkpoints.enough_checkpoints()){
        //no enough checkpoints added for a race
        m.message_no_args(this, "Error", "Not enough checkpoints.\n A race must have at least 5 checkpoints.");
    }

    if (checkpoints.export_checkpoints(map)){
        //file create succesfully
        m.message_no_args(this, "Success", "Congratulations! Creation complete");
        emit close_w();
        this->close();
    }else{
        m.message_no_args(this, "Error", "An error has occured.\n The circuit has not been save.");
    }

}

