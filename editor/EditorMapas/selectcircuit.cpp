#include "selectcircuit.h"

#include <QDir>
#include <QMouseEvent>

#include "creador.h"
#include "race.h"
#include "ui_selectcircuit.h"

SelectCircuit::SelectCircuit(const QString& path, QWidget* parent):
        QMainWindow(parent), ui(new Ui::SelectCircuit), path_races(path) {
    ui->setupUi(this);

    fill_table();
}

void SelectCircuit::fill_table() {
    QDir dir(path_races);

    // races names
    QStringList races_names = dir.entryList(QStringList() << "*.csv", QDir::Files);
    qDebug() << "size: " << races_names.size();

    ui->tableRaces->clearContents();
    ui->tableRaces->setRowCount(races_names.length());

    QStringList col = {"Name", "City", "# Checkpoints", "# laps", "Open", "Delete"};
    ui->tableRaces->setColumnCount(col.size());
    ui->tableRaces->setHorizontalHeaderLabels(col);

    ui->tableRaces->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableRaces->setShowGrid(false);
    ui->tableRaces->verticalHeader()->setVisible(false);
    ui->tableRaces->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableRaces->setFocusPolicy(Qt::NoFocus);
    ui->tableRaces->setSelectionMode(QAbstractItemView::NoSelection);

    read_info_races(races_names);

    fill_races_info();
}

void SelectCircuit::fill_races_info() {
    for (const Race& r: this->races) {
        int id = r.id;
        QString name = r.name;
        QString city = r.city;
        int n_c = r.n_checkpoints;
        int n_l = r.n_laps;
        ui->tableRaces->setItem(id, 0, new QTableWidgetItem(name));
        ui->tableRaces->setItem(id, 1, new QTableWidgetItem(city));
        ui->tableRaces->setItem(id, 2, new QTableWidgetItem(QString::number(n_c)));
        ui->tableRaces->setItem(id, 3, new QTableWidgetItem(QString::number(n_l)));

        QPushButton* open_btn = new QPushButton("Open");
        open_btn->setStyleSheet(
                "background-color: green; color: white; font-weight: bold;min-height: 12px;");

        connect(open_btn, &QPushButton::clicked, this, [this, r]() { select_race(r); });

        ui->tableRaces->setCellWidget(id, 4, open_btn);

        QPushButton* btn = new QPushButton("Delete");
        btn->setStyleSheet(
                "background-color: red; color: white; font-weight: bold;min-height: 12px;");

        connect(btn, &QPushButton::clicked, this, [this, name]() { delete_race(name); });

        ui->tableRaces->setCellWidget(id, 5, btn);
    }
}

void SelectCircuit::read_info_races(const QStringList& races_names) {
    int id = 0;
    for (const QString& r: races_names) {
        QString filePath = path_races + "/" + r;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        QString baseName = QFileInfo(filePath).baseName();

        QTextStream in(&file);

        QString firstline = in.readLine();
        QStringList images = firstline.split(",");
        QString map = images[0];
        QString mask = images[1];

        in.readLine();  // skip header City,n_laps

        QString info = in.readLine();

        QStringList infoRace = info.split(',');
        QString cityName = infoRace[0];
        QString number_laps = infoRace[1];
        int n_laps = number_laps.toInt();


        QString header = in.readLine();
        if (header.size() > 0) {
            // header skip
        }


        QVector<Checkpoint> checkpoints;
        int checkpointCount = 0;

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed().isEmpty())
                continue;

            QStringList cols = line.split(',');
            if (cols.size() < 6)
                continue;

            int id_checkpoint = cols[0].toInt();
            char orientation = cols[1].toStdString()[0];
            int xCenter = cols[2].toInt();
            int yCenter = cols[3].toInt();
            int width = cols[4].toInt();
            int height = cols[5].toInt();

            // QRect center in (xCenter, yCenter)
            QRect rect(xCenter - width / 2, yCenter - height / 2, width, height);

            Checkpoint cp(id_checkpoint, orientation, QPoint(xCenter, yCenter), rect);
            checkpoints.append(cp);
            checkpointCount++;
        }

        file.close();

        // create a race
        Race race(id, baseName, cityName, map, mask, checkpointCount, checkpoints, n_laps);
        id++;

        races.append(race);
    }
}

SelectCircuit::~SelectCircuit() { delete ui; }

const QString& SelectCircuit::races_folder() const { return path_races; }

void SelectCircuit::on_pushButton_clicked() {
    emit close_select_race();
    this->close();
}

void SelectCircuit::delete_race(const QString& race_name) {

    QFile file(QDir(path_races).filePath(race_name + ".csv"));
    if (file.exists()) {
        file.remove();
        races.clear();
        fill_table();
    }
}

void SelectCircuit::select_race(const Race& race) {
    this->hide();
    Creador* c = new Creador(race, path_races);
    c->setAttribute(Qt::WA_DeleteOnClose);

    connect(c, &Creador::close_w, this, [this]() {
        races.clear();
        fill_table();
        this->show();
    });

    c->showMaximized();
}
