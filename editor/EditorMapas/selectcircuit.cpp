#include "selectcircuit.h"
#include "ui_selectcircuit.h"

#include <QDir>
#include <QMouseEvent>
#include "race.h"
#include "creador.h"

SelectCircuit::SelectCircuit(const QString& path, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SelectCircuit)
    , path_races(path)
{
    ui->setupUi(this);

    fill_table();

}

void SelectCircuit::fill_table(){
    QDir dir(path_races);

    //races names
    QStringList races_names = dir.entryList(QStringList() << "*.csv", QDir::Files);

    ui->tableRaces->clear();
    ui->tableRaces->setRowCount(races_names.length());

    QStringList col = {"Name", "City", "# Checkpoints", "Actions"};
    ui->tableRaces->setColumnCount(col.size());
    ui->tableRaces->setHorizontalHeaderLabels(col);

    ui->tableRaces->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableRaces->setShowGrid(false);
    ui->tableRaces->verticalHeader()->setVisible(false);

    read_info_races(races_names);

    fill_races_info();
}

void SelectCircuit::fill_races_info(){
    for (Race& r:this->races){
        int id = r.id;
        QString name = r.name;
        QString city = r.city;
        int n_c = r.n_checkpoints;

        ui->tableRaces->setItem(id, 0, new QTableWidgetItem(name));
        ui->tableRaces->setItem(id, 1, new QTableWidgetItem(city));
        ui->tableRaces->setItem(id, 2, new QTableWidgetItem(QString::number(n_c)));

        QPushButton* btn = new QPushButton("Delete");
        btn->setStyleSheet("background-color: red; color: white; font-weight: bold;min-height: 12px;");

        connect(btn, &QPushButton::clicked, this, [this, name]() {
            delete_race(name);
        });

        ui->tableRaces->setCellWidget(id, 3, btn);
    }
}

void SelectCircuit::read_info_races(const QStringList& races_names){
    int id = 0;
    for (const QString& r: races_names){
        QString filePath = path_races + "/" + r;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        QString baseName = QFileInfo(filePath).baseName();

        QTextStream in(&file);

        QString line = in.readLine();
        QStringList images = line.split(",");
        QString map = images[0];
        QString mask = images[1];

        QString header = in.readLine();

        QStringList headerCols = header.split(',');
        QString cityName;
        if (!headerCols.isEmpty()) {
            cityName = headerCols.last().trimmed();
        }

        QVector<Checkpoint> checkpoints;
        int checkpointCount = 0;

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed().isEmpty()) continue;

            QStringList cols = line.split(',');
            if (cols.size() < 6) continue;

            int id_checkpoint = cols[0].toInt();
            char orientation = cols[1].toStdString()[0];
            int xCenter = cols[2].toInt();
            int yCenter = cols[3].toInt();
            int height = cols[4].toInt();
            int width = cols[5].toInt();

            // QRect center in (xCenter, yCenter)
            QRect rect(xCenter - width/2, yCenter - height/2, width, height);

            Checkpoint cp(id_checkpoint,orientation, QPoint(xCenter, yCenter), rect);
            checkpoints.append(cp);
            checkpointCount++;
        }

        file.close();

        // create a race
        Race race(id, baseName, cityName,map,mask, checkpointCount, checkpoints);
        id++;

        races.append(race);
    }
}

SelectCircuit::~SelectCircuit()
{
    delete ui;
}

void SelectCircuit::on_pushButton_clicked()
{
    emit close_select_race();
    this->close();
}

void SelectCircuit::delete_race(const QString& race_name){

    QString path = QCoreApplication::applicationDirPath() + "/../server/races/" + race_name + ".csv";
    QFile file(path);
    if (file.exists()) {
        file.remove();
        races.clear();
        fill_table();
    }
}

void SelectCircuit::on_tableRaces_cellDoubleClicked(int row, int column)
{
    if(column != 0){
        //no click in a race name
        return;
    }

    //open the race, initialize the checkpoints, can edit the race again
    QTableWidgetItem* cell = ui->tableRaces->item(row,column);
    QString race_name = cell->text();

    int cont = 0;
    bool found = false;

    for (const Race& r : races) {
        if (race_name == r.name) {
            found = true;
            break;
        }
        cont++;
    }

    if (!found) {
        return;
    }

    Race race_click = races[cont];

    this->hide();
    Creador *c = new Creador(race_click);
    c->setAttribute(Qt::WA_DeleteOnClose);

    connect(c, &Creador::close_w, this, [this]() {
        races.clear();
        fill_table();
        this->show();
    });

    c->showMaximized();
}
