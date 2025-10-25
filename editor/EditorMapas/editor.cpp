#include "editor.h"
#include "ui_editor.h"
#include "creador.h"

Editor::Editor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Editor)
{
    ui->setupUi(this);
}

Editor::~Editor()
{
    delete ui;
}

void Editor::on_mapa1_clicked()
{
    QString mapa = ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Liberty City.png";
    //se usara el mapa de liberty city
    //se cierra la ventana de seleccion de circuito
    this->hide();
    //se abre el form con el circuito de liberty city
    Creador *c = new Creador(mapa);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // Conectar la señal "cerrado" de Creador a un slot lambda que muestre Editor
    connect(c, &Creador::close_w, this, [this]() {
        this->show(); // cuando Creador se cierre, mostramos Editor
    });

    c->showMaximized();
}


void Editor::on_mapa2_clicked()
{
//se usara el mapa de San Andreas
    QString mapa = ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - San Andreas.png";
    //se cierra la ventana de seleccion de circuito
    this->hide();
    //se abre el form con el circuito de san andreas
    Creador *c = new Creador(mapa);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // Conectar la señal "close" de Creador a un slot lambda que muestre Editor
    connect(c, &Creador::close_w, this, [this]() {
        this->show(); // cuando Creador se cierre, mostramos Editor
    });

    c->show();
}


void Editor::on_mapa3_clicked()
{
//se usara el mapa de Vice City
    QString mapa = ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Vice City.png";
    //se cierra la ventana de seleccion de circuito
    this->hide();
    //se abre el form con el circuito de vice city
    Creador *c = new Creador(mapa);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // Conectar la señal "cerrado" de Creador a un slot lambda que muestre Editor
    connect(c, &Creador::close_w, this, [this]() {
        this->show(); // cuando Creador se cierre, mostramos Editor
    });

    c->show();
}




void Editor::on_Exit_clicked()
{
    QApplication::quit();
}

