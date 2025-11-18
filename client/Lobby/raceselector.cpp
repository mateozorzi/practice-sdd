#include "raceselector.h"

#include "ui_raceselector.h"
#include "../../common/race_catalogue_dto.h"
#include "../../common/race_dto.h"
#include <QMessageBox>


RaceSelector::RaceSelector(RaceCatalogueDTO& races, QWidget* parent):
        QMainWindow(parent), ui(new Ui::RaceSelector), races(races) {
    ui->setupUi(this);

    available_races = this->races.races;
    selected_races.clear();

    fill_available_races();
    config_available_races();
    config_select_races();
}

void RaceSelector::fill_available_races(){
    ui->availableRaces->clear();

    ui->availableRaces->setRowCount(static_cast<int>(available_races.size()));

    config_available_races();

    for (int i = 0; i < static_cast<int>(available_races.size()); ++i) {
        const RaceDTO& r = available_races[i];

        ui->availableRaces->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.name)));
        ui->availableRaces->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(r.city)));
        ui->availableRaces->setItem(i, 2, new QTableWidgetItem(QString::number(r.size_checkpoints)));
        ui->availableRaces->setItem(i, 3, new QTableWidgetItem(QString::number(r.laps)));

        QPushButton* btn = new QPushButton("Add");
        btn->setStyleSheet("background-color: green; color: white; font-weight: bold;");

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            select_race(i);
        });

        ui->availableRaces->setCellWidget(i, 4, btn);
    }
}

void RaceSelector::config_select_races(){
    QStringList col = {"Name", "City", "# Checkpoints", "# laps", "Actions"};
    ui->select->setColumnCount(col.size());
    ui->select->setHorizontalHeaderLabels(col);

    ui->select->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->select->setShowGrid(false);
    ui->select->verticalHeader()->setVisible(false);
}

void RaceSelector::config_available_races(){
    QStringList col = {"Name", "City", "# Checkpoints", "# laps", "Actions"};
    ui->availableRaces->setColumnCount(col.size());
    ui->availableRaces->setHorizontalHeaderLabels(col);

    ui->availableRaces->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->availableRaces->setShowGrid(false);
    ui->availableRaces->verticalHeader()->setVisible(false);
}

void RaceSelector::select_race(int race_index) {
    if (race_index < 0 || static_cast<size_t>(race_index) >= available_races.size()) return;

    RaceDTO race = available_races[race_index];
    available_races.erase(available_races.begin() + race_index);
    selected_races.push_back(race);

    refresh_available_table();
    refresh_selected_table();

}

void RaceSelector::delete_race(int race_index) {
    if (race_index < 0 || static_cast<size_t>(race_index) >= selected_races.size()) return;

    RaceDTO race = selected_races[race_index];
    selected_races.erase(selected_races.begin() + race_index);
    available_races.push_back(race);

    refresh_available_table();
    refresh_selected_table();
}

void RaceSelector::refresh_available_table() {
    ui->availableRaces->clear();
    ui->availableRaces->setRowCount(static_cast<int>(available_races.size()));

    config_available_races();

    for (int i = 0; i < static_cast<int>(available_races.size()); ++i) {
        const RaceDTO& r = available_races[i];

        ui->availableRaces->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.name)));
        ui->availableRaces->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(r.city)));
        ui->availableRaces->setItem(i, 2, new QTableWidgetItem(QString::number(r.size_checkpoints)));
        ui->availableRaces->setItem(i, 3, new QTableWidgetItem(QString::number(r.laps)));

        QPushButton* btn = new QPushButton("Add");
        btn->setStyleSheet("background-color: green; color: white; font-weight: bold;");

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            select_race(i);
        });

        ui->availableRaces->setCellWidget(i, 4, btn);
    }
}

void RaceSelector::refresh_selected_table() {
    ui->select->clear();
    ui->select->setRowCount(static_cast<int>(selected_races.size()));

    config_select_races();

    for (int i = 0; i < static_cast<int>(selected_races.size()); ++i) {
        const RaceDTO& r = selected_races[i];

        ui->select->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.name)));
        ui->select->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(r.city)));
        ui->select->setItem(i, 2, new QTableWidgetItem(QString::number(r.size_checkpoints)));
        ui->select->setItem(i, 3, new QTableWidgetItem(QString::number(r.laps)));

        QPushButton* btn = new QPushButton("Delete");
        btn->setStyleSheet("background-color: red; color: white; font-weight: bold;");

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            delete_race(i);
        });

        ui->select->setCellWidget(i, 4, btn);
    }
}

void RaceSelector::on_pushButton_2_clicked()
{
    emit close_menu();
}


void RaceSelector::on_pushButton_clicked()
{
    //selected_races size >= 1
    if(selected_races.size() < 1){
        QMessageBox::warning(this, "Error", "There must be at least one race selected.");
        return;
    }

    emit confirm_races(selected_races);
}


RaceSelector::~RaceSelector() { delete ui; }
