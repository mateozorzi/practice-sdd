#include "selectcars.h"

#include "ui_selectcars.h"
#include "../../common/car_dto.h"

SelectCars::SelectCars(QWidget* parent): QMainWindow(parent), ui(new Ui::SelectCars) {
    ui->setupUi(this);
    scene = new QGraphicsScene(this);

    ui->Carinfo->setRowCount(5);
    ui->Carinfo->setColumnCount(1);
    QStringList rowLabels;
    rowLabels << "Model" << "Health" << "Handling" << "Acceleration" << "Speed";
    ui->Carinfo->setVerticalHeaderLabels(rowLabels);
    QStringList colLabels;
    colLabels << "Value";
    ui->Carinfo->setHorizontalHeaderLabels(colLabels);
    ui->Carinfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rows_format();
    value_format();

    connect(ui->combobox, &QComboBox::currentIndexChanged, this, &SelectCars::onCarSelected);

}

SelectCars::~SelectCars() {
    delete ui;
    delete scene;
}

void SelectCars::on_Confirm_clicked()
{
    QString select_car = ui->combobox->currentText();
    emit confirm_car(select_car);
    this->close();
}


void SelectCars::on_Exit_clicked()
{
    emit back_to_lobby();
    this->close();
}

void SelectCars::onCarSelected(int index){
    const car_dto& car = market[index];

    QString path = ":/cars/images/" + QString::fromStdString(car.model) + ".png";
    scene->clear();
    scene->addPixmap(QPixmap(path));
    ui->Carimage->setScene(scene);
    ui->Carimage->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    ui->Carinfo->clearContents();
    ui->Carinfo->setItem(0, 0, new QTableWidgetItem(QString::fromStdString(car.model)));
    ui->Carinfo->setItem(1, 0, new QTableWidgetItem(QString::number(car.health)));
    ui->Carinfo->setItem(2, 0, new QTableWidgetItem(QString::number(car.handling)));
    ui->Carinfo->setItem(3, 0, new QTableWidgetItem(QString::number(car.acceleration)));
    ui->Carinfo->setItem(4, 0, new QTableWidgetItem(QString::number(car.speed)));
    value_format();
}

void SelectCars::rows_format(){
    QString headerStyle = R"(
    QHeaderView::section {
        background-color: black;
        color: white;
        font-weight: bold;
        font-size: 25pt;
        text-align: center;
        padding: 8px;
    }
    )";
    ui->Carinfo->verticalHeader()->setStyleSheet(headerStyle);
}

void SelectCars::value_format(){
    QFont font;
    font.setPointSize(24);

    int column = 0;
    for (int row = 0; row < ui->Carinfo->rowCount(); ++row) {
        QTableWidgetItem* item = ui->Carinfo->item(row, column);
        if (item) {
            item->setBackground(Qt::black);
            item->setForeground(Qt::white);
            item->setFont(font);
            item->setTextAlignment(Qt::AlignCenter);
        }
    }
}

void SelectCars::set_market(const std::vector<car_dto>& cars){
    market = cars;

    for (const car_dto& car: cars){
        QString name = QString::fromStdString(car.model);
        ui->combobox->addItem(name);
    }
}

