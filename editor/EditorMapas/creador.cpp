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

#define RADIO 8
#define MIN_CHECKPOINTS 5


Creador::Creador(const QString& path, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Creador)
    , map(path)
    , checkpoints(Coordinates())
    , mode(ADD)
    , m(Messages())
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->labelmap->setScene(scene);

    set_background(map);
    ui->n_checkpoints->display(0);


    ui->labelmap->installEventFilter(this);


    ui->addButton->setCheckable(true);
    ui->removeButton->setCheckable(true);
    connect(ui->addButton, &QPushButton::toggled, this, &Creador::on_addButton_toggled);
    connect(ui->removeButton, &QPushButton::toggled, this, &Creador::on_removeButton_toggled);

    ui->addButton->setChecked(true);
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


void Creador::update_map(const QPoint& pos){
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

void Creador::add_checkpoint(QEvent *event){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    QPointF scenePos = ui->labelmap->mapToScene(mouseEvent->pos());
    QPoint pos = scenePos.toPoint();


    if(checkpoints.add_checkpoint(pos)){
        update_map(pos);
        m.checkpoint_message(this,"Success", "Checkpoint added in (%1, %2)", pos);

    }else{
        m.checkpoint_message(this, "Error", "Can´t add checkpoint in (%1, %2)", pos);
    }
}

void Creador::delete_checkpoint(QEvent *event){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    QPointF scenePos = ui->labelmap->mapToScene(mouseEvent->pos());
    QPoint pos = scenePos.toPoint();

    QPoint del = checkpoints.delete_checkpoint(pos);

    if(del.x() != -1){
        update_map(del);
        m.checkpoint_message(this, "Success", "Checkpoint delete in (%1, %2)", pos);

    }else{
        m.checkpoint_message(this, "Error", "No checkpoint in (%1, %2)", pos);
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
    qDeleteAll(scene->items());
    delete scene;
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

void Creador::crear_csv(){
    QString path = QCoreApplication::applicationDirPath() + "/../../races";
    QDir dir(path);

    if (!dir.exists() && !dir.mkpath(".")) {
        m.message_no_args(this, "Error", "No se pudo crear la carpeta 'races'");
        return;
    }

    // Usar QFileInfo para obtener el nombre base sin extensión
    QFileInfo fileInfo(map);
    QString baseName = fileInfo.baseName();  // Esto devuelve el nombre sin extensión

    QString race_name = baseName + ".csv";
    QString path_race = dir.filePath(race_name);

    int id = 1;
    while (QFile::exists(path_race)) {
        race_name = baseName + "_" + QString::number(id) + ".csv";
        path_race = dir.filePath(race_name);
        id++;
    }
    m.message_no_args(this,path_race, race_name);
    QFile archivo(path_race);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m.message_no_args(this, "Error", "Can´t open file: " + archivo.errorString());
        return;
    }

    QTextStream out(&archivo);
    out << "Nombre de la carrera: " << baseName << "\n";
    out << "ID de la carrera: " << id << "\n";

    m.message_no_args(this, "Success", "Congratulations! Creation complete");
}
void Creador::on_Done_clicked()
{
    //create csv with the checkpoints of the race

    //a race must have at least 5 checkpoints
    if (checkpoints.total_checkpoints() < MIN_CHECKPOINTS){
        m.message_no_args(this, "Error", "Not enough checkpoints.\n A race must have at least 5 checkpoints.");
    }else{
        crear_csv();

        emit close_w();
        this->close();
    }

}

