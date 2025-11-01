#include "selectcircuit.h"
#include "ui_selectcircuit.h"

#include <QDir>
#include "race.h"

SelectCircuit::SelectCircuit(const QString& path, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SelectCircuit)
    , path_races(path)
{
    ui->setupUi(this);

    //set up the table with the races available
    QDir dir(path_races);

    //races names
    QStringList races_names = dir.entryList(QStringList() << "*.csv", QDir::Files);

    ui->tableRaces->setRowCount(races_names.length());

    ui->tableRaces->setVerticalHeaderLabels(races_names);
    QStringList col = {"City", "Number of checkpoints"};
    ui->tableRaces->setColumnCount(col.size());
    ui->tableRaces->setHorizontalHeaderLabels(col);


    read_info_races(races_names);

    fill_races_info();

}

void SelectCircuit::fill_races_info(){
    for (Race& r:this->races){
        int id = r.id;
        QString city = r.city;
        int n_c = r.n_checkpoints;

        ui->tableRaces->setItem(id, 0, new QTableWidgetItem(city));
        ui->tableRaces->setItem(id, 1, new QTableWidgetItem(QString::number(n_c)));
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

        QTextStream in(&file);
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
        Race race(id, r, cityName, checkpointCount, checkpoints);
        id++;

        // Guardamos o hacemos lo que necesites con `race`
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


void SelectCircuit::on_tableRaces_cellDoubleClicked(int row, int column)
{
    if(row != 0){
        //no click in a race name
        return;
    }

    //open the race, initialize the checkpoints, can edit the race again
}

