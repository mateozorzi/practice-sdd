#include "editor.h"

#include "creador.h"
#include "races_folder.h"
#include "ui_editor.h"

Editor::Editor(QWidget* parent): QMainWindow(parent), ui(new Ui::Editor) { ui->setupUi(this); }

Editor::~Editor() { delete ui; }

void Editor::on_mapa1_clicked() {
    QString mapa = ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Liberty "
                   "City.png";
    QString mask = ":/mask/imagenes/001 - B_STREETS - Liberty City.png";
    // liberty city map
    this->hide();
    Creador* c = new Creador(mapa, mask, RACES_FOLDER);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Creador::close_w, this, [this]() {
        emit close_editor();
        this->close();
    });

    c->showMaximized();
}


void Editor::on_mapa2_clicked() {
    QString mapa = ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - San "
                   "Andreas.png";
    QString mask = ":/mask/imagenes/002 - B_STREETS - San Andreas.png";
    // san andreas amp
    this->hide();
    Creador* c = new Creador(mapa, mask, RACES_FOLDER);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Creador::close_w, this, [this]() {
        emit close_editor();
        this->close();
    });

    c->showMaximized();
}


void Editor::on_mapa3_clicked() {
    QString mapa =
            ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Vice City.png";
    QString mask = ":/mask/imagenes/003 - B_STREETS - Vice City.png";
    // vice city map
    this->hide();
    Creador* c = new Creador(mapa, mask, RACES_FOLDER);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Creador::close_w, this, [this]() {
        emit close_editor();
        this->close();
    });

    c->showMaximized();
}


void Editor::on_Exit_clicked() {
    emit close_editor();
    this->close();
}
