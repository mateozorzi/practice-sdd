#include "editormenu.h"
#include "ui_editormenu.h"
#include "editor.h"
#include "selectcircuit.h"

EditorMenu::EditorMenu(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EditorMenu)
{
    ui->setupUi(this);
}

EditorMenu::~EditorMenu()
{
    delete ui;
}

void EditorMenu::on_pushButton_2_clicked()
{
    //open the creator
    this->hide();
    Editor *c = new Editor();
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &Editor::close_editor, this, [this]() {
        this->show();
    });

    c->show();
}


void EditorMenu::on_pushButton_3_clicked()
{
    QApplication::quit();
}


void EditorMenu::on_pushButton_clicked()
{
    //edit a already existing race
    this->hide();
    SelectCircuit *c = new SelectCircuit("../server/races");
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &SelectCircuit::close_select_race, this, [this]() {
        this->show();
    });

    c->show();
}

