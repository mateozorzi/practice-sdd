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
#include "checkpoint.h"
#include "Constants.h"

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

Creador::Creador(const Race& r, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Creador)
    , map(r.map)
    , mask(r.mask)
    , checkpoints(r.checkpoints)
    , m(Messages()){

    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->labelmap->setScene(scene);

    set_background(map);

    mask_circuit.load(mask);

    ui->n_checkpoints->display(r.n_checkpoints);
    ui->name_circuit->setText(r.name);

    ui->labelmap->installEventFilter(this);

    import_checkpoints();
}

void Creador::import_checkpoints(){
    //draw checkpoints on the map
    QVector<Checkpoint> v_checkpoints = checkpoints.get_checkpoints();

    for (Checkpoint c: v_checkpoints){
        QRect r = c.rect;
        scene->addRect(r,
                       QPen(Qt::red), QBrush(Qt::yellow));
    }
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
        if (dynamic_cast<QGraphicsRectItem*>(item)) {
            count++;
        }
    }
    ui->n_checkpoints->display(count);
}

Checkpoint Creador::calculate_checkpoint(const QPoint& pos){
    int id = static_cast<int>(ui->n_checkpoints->value() +1);
    int left = pos.x(), right = pos.x();
    while (left > 0 && is_street(QPoint(left - 1, pos.y()))) left--;
    while (right < mask_circuit.width() - 1 && is_street(QPoint(right + 1, pos.y()))) right++;
    int horizontalLength = right - left;


    int up = pos.y(), down = pos.y();
    while (up > 0 && is_street(QPoint(pos.x(), up - 1))) up--;
    while (down < mask_circuit.height() - 1 && is_street(QPoint(pos.x(), down + 1))) down++;
    int verticalLength = down - up;

    if(horizontalLength >= MAX_RECT && verticalLength >= MAX_RECT){
        //intersection checkpoint
        QRect rect(pos.x() - INTERSECTION_SIZE / 2,
                   pos.y() - INTERSECTION_SIZE / 2,
                   INTERSECTION_SIZE, INTERSECTION_SIZE);
        return Checkpoint(id,INTERSECTION, pos, rect);

    }else if (horizontalLength < verticalLength) {
        // horizontal checkpoint
        int height = 8;

        QRect rect = QRect(left, pos.y() - height/2, horizontalLength, height);
        return Checkpoint(id,HORIZONTAL, pos, rect);
    } else {
        // vertical checkpoint
        int width = 8;

        QRect rect = QRect(pos.x() - width/2, up, width, verticalLength);
        return Checkpoint(id,VERTICAL, pos, rect);
    }

}


void Creador::update_map(const QPoint& pos, int mode){
    if (mode == ADD){
        Checkpoint checkpoint = calculate_checkpoint(pos);
        if(checkpoints.add_checkpoint(checkpoint)){
            scene->addRect(checkpoint.get_rect(),
                                                QPen(Qt::red), QBrush(Qt::yellow));

        }else{
            m.checkpoint_message(this, "Error", "Can´t add checkpoint in (%1, %2)", pos);
        }

    }else if (mode == REMOVE){
        QPointF clickPos = pos;

        for (QGraphicsItem* item : scene->items()) {
            if (auto rect = dynamic_cast<QGraphicsRectItem*>(item)) {
                if(rect->contains(clickPos)){
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
    QColor streetColor(255, 36, 250);

    QColor pixelColor = mask_circuit.pixelColor(pos);

    int tolerance = 30;
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

    if(is_street(pos)){
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
        return true;
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
        return;
    }

    QString name_circuit = ui->name_circuit->text();
    if (name_circuit.length() < MIN_LENGTH){
        m.message_no_args(this, "Error", "Fill the name of the circuit.\n The name must have at least five letters");
        return;
    }

    if (checkpoints.export_checkpoints(name_circuit, map, this->mask, this)){
        //file create succesfully
        m.message_no_args(this, "Success", "Congratulations! Creation complete");
        emit close_w();
        this->close();
    }else{
        m.message_no_args(this, "Error", "An error has occured.\n The circuit has not been save.");
    }

}

