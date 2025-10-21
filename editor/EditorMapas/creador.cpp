#include "creador.h"
#include "ui_creador.h"

Creador::Creador(const QString& path, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Creador)
    , mapa(path)
{
    ui->setupUi(this);
    colocar_fondo(mapa);
}

void Creador::colocar_fondo(const QString& path){
    QPixmap mapa(path);
    if(!mapa.isNull()) {
        // Escalar la imagen al tamaño del label
        ui->labelmapa->setPixmap(mapa.scaled(
            ui->labelmapa->size(),
            Qt::KeepAspectRatio,  // mantiene proporción
            Qt::SmoothTransformation
            ));
    }
}

Creador::~Creador()
{
    delete ui;
}

void Creador::on_pushButton_clicked()
{
    emit cerrado();
    this->close();
}

