#include "editormenu.h"

#include <QCoreApplication>
#include <QDir>

#include "editor.h"
#include "races_folder.h"
#include "selectcircuit.h"
#include "ui_editormenu.h"

const QString PROJECT_ROOT_NAME = "TP_Taller_de_Programacion-Need-For-Speed";

EditorMenu::EditorMenu(QWidget* parent): QMainWindow(parent), ui(new Ui::EditorMenu) {
    ui->setupUi(this);
}

EditorMenu::~EditorMenu() { delete ui; }

void EditorMenu::on_pushButton_2_clicked() {
    // open the creator
    this->hide();
    Editor* c = new Editor();
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Editor::close_editor, this, [this]() { this->show(); });

    c->showMaximized();
}


void EditorMenu::on_pushButton_3_clicked() { QApplication::quit(); }


void EditorMenu::on_pushButton_clicked() {
    // edit a already existing race
    this->hide();

    SelectCircuit* c = new SelectCircuit(RACES_FOLDER);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &SelectCircuit::close_select_race, this, [this]() { this->show(); });

    c->showMaximized();
}
