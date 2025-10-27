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
    QString mask = ":/mask/imagenes/001_B_STREETS_liberty_city.png";
    //liberty city map
    this->hide();
    Creador *c = new Creador(mapa, mask);
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
    QString mask = ":/mask/imagenes/002_B_STREETS_san_andreas.png";
    //san andreas amp
    this->hide();
    Creador *c = new Creador(mapa, mask);
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
    QString mask = ":/mask/imagenes/003_B_STREETS_vice_city.png";
    //vice city map
    this->hide();
    Creador *c = new Creador(mapa, mask);
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

