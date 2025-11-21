#include "editormenu.h"
#include "ui_editormenu.h"
#include "editor.h"
#include "selectcircuit.h"

#include <QDir>
#include <QCoreApplication>

const QString PROJECT_ROOT_NAME = "TP_Taller_de_Programacion-Need-For-Speed";

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

    c->showMaximized();
}


void EditorMenu::on_pushButton_3_clicked()
{
    QApplication::quit();
}


QString EditorMenu::find_races_path() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    qDebug() << "path: "<<dir.path();
    //dir.cdUp();
    while (true) {
        if (dir.exists("server")) {
            dir.cd("server");
            if (!dir.exists("races")) {
                dir.mkdir("races");
            }
            dir.cd("races");
            qDebug() << "path: "<<dir.path();
            return dir.path();
        }

        if (dir.dirName() == PROJECT_ROOT_NAME){
            break;
        }

        if (!dir.cdUp()) {
            qDebug() << "path: "<<dir.path();
            break;
        }
    }
    return QString();
}

void EditorMenu::on_pushButton_clicked()
{
    //edit a already existing race
    this->hide();

    QString races_path = find_races_path();
    SelectCircuit *c = new SelectCircuit(races_path);
    c->setAttribute(Qt::WA_DeleteOnClose);

    // signal to know when the editor has close
    connect(c, &SelectCircuit::close_select_race, this, [this]() {
        this->show();
    });

    c->showMaximized();
}

