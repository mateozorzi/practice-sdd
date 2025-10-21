#include "creador.h"
#include "ui_creador.h"
#include "coordinates.h"
#include <QMouseEvent>
#include <QMessageBox>


Creador::Creador(const QString& path, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Creador)
    , map(path)
    , checkpoints(Coordinates())
{
    ui->setupUi(this);
    set_background(map);

    ui->labelmap->installEventFilter(this);
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

void Creador::handle_checkpoints(QEvent *event){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    // Posición dentro del label
    QPoint pos = mouseEvent->pos();

    // Guardar coordenada
    checkpoints.add_checkpoint(pos);

    QMessageBox::information(this, "Checkpoint agregado",
                             QString("Se agregó un checkpoint en (%1, %2)")
                                 .arg(pos.x()).arg(pos.y()));

    // Dibujar el punto sobre el mapa
    //actualizarMapa();
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

