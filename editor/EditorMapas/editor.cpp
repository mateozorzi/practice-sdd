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
    //liberty city map
    this->hide();
    Creador *c = new Creador(mapa);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Creador::close_w, this, [this]() {
        this->show();
    });

    c->showMaximized();
}


void Editor::on_mapa2_clicked()
{
    QString mapa = ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - San Andreas.png";
    //san andreas amp
    this->hide();
    Creador *c = new Creador(mapa);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Creador::close_w, this, [this]() {
        this->show();
    });

    c->show();
}


void Editor::on_mapa3_clicked()
{
    QString mapa = ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Vice City.png";
    //vice city map
    this->hide();
    Creador *c = new Creador(mapa);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Creador::close_w, this, [this]() {
        this->show();
    });

    c->show();
}




void Editor::on_Exit_clicked()
{
    QApplication::quit();
}

