#include "editor.h"
#include "./ui_editor.h"

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

void Editor::accionmapa1(){

}

void Editor::on_mapa1_clicked()
{

}


void Editor::on_mapa2_clicked()
{

}


void Editor::on_mapa3_clicked()
{

}

