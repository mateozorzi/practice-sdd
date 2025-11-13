#include "selectcars.h"

#include "ui_selectcars.h"
#include "../../common/car_dto.h"
#include "../../common/car_catalogue_dto.h"
#include <QLabel>
#include <QProgressBar>

#define MAX_IMG 64

SelectCars::SelectCars(CarCatalogueDTO& catalogue, QWidget* parent):
        QMainWindow(parent)
        , ui(new Ui::SelectCars)
        , catalogue(catalogue)
        , actual_car(car_dto())
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);

    value_format();
    QStringList colLabels;
    colLabels <<"Name" << "Health" << "Handling" << "Acceleration" << "Speed";
    ui->Carinfo->setRowCount(colLabels.size());
    ui->Carinfo->setColumnCount(1);
    ui->Carinfo->setVerticalHeaderLabels(colLabels);
    ui->Carinfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Carinfo->verticalHeader()->setVisible(true);
    ui->Carinfo->horizontalHeader()->setVisible(false);

    for (int row = 0; row < 5; ++row) {
        ui->Carinfo->verticalHeader()->setSectionResizeMode(row, QHeaderView::Stretch);
    }

    ui->Carinfo->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    set_market();

    onCarChosen(catalogue.cars[0]);

}

SelectCars::~SelectCars() {
    delete ui;
    delete scene;
}

void SelectCars::on_Exit_clicked()
{
    emit back_to_lobby();
    this->close();
}

void SelectCars::value_format(){
    QString style = R"(
    QTableWidget#Carinfo {
        background-color: transparent;
        alternate-background-color: transparent;
    }

    QTableWidget#Carinfo::item {
        color: black;
        font-size: 14pt;
        border: 1px solid black;
        background-color: white;
        text-align: center;
    }

    QTableWidget#Carinfo::item:selected {
        background-color: #e0e0e0;
        color: black;
    }

    QHeaderView::section {
        color: white;
        font-size: 25pt;
        font-weight: bold;
        background-color: black;
        border: 1px solid black;
        text-align: center;
    }

    QTableWidget#Carinfo QLabel {
        background-color: white;
        border: none;
        font-size: 14pt;
    }
)";

    ui->Carinfo->setStyleSheet(style);
}

car_dto SelectCars::inspect_catalogue() {
    int maxHealth = 0;
    int maxHandling = 0;
    int maxAcceleration = 0;
    int maxSpeed = 0;

    for (const car_dto& car : catalogue.cars) {
        maxHealth = std::max(maxHealth, (int)car.health);
        maxHandling = std::max(maxHandling, (int)car.handling);
        maxAcceleration = std::max(maxAcceleration, (int)car.acceleration);
        maxSpeed = std::max(maxSpeed, (int)car.speed);
    }

    uint8_t Health = static_cast<uint8_t>(maxHealth);
    uint8_t Handling = static_cast<uint8_t>(maxHandling);
    uint8_t Acceleration = static_cast<uint8_t>(maxAcceleration);
    uint8_t Speed = static_cast<uint8_t>(maxSpeed);

    return car_dto("Normalized", Health, Handling, Speed, Acceleration);
}

car_dto SelectCars::normalize_value_car(const car_dto& car, const car_dto& normalized) {
    int health_normalized = 0;
    int handling_normalized = 0;
    int accelaration_normalized = 0;
    int speed_normalized = 0;

    if (static_cast<int>(normalized.health) > 0){
        health_normalized = (static_cast<int>(car.health) * 100) / static_cast<int>(normalized.health);
    }

    if (static_cast<int>(normalized.handling) > 0){
        handling_normalized = (static_cast<int>(car.handling) * 100) / static_cast<int>(normalized.handling);
    }

    if (static_cast<int>(normalized.acceleration) > 0){
        accelaration_normalized = (static_cast<int>(car.acceleration) * 100) / static_cast<int>(normalized.acceleration);
    }

    if (static_cast<int>(normalized.speed) > 0){
        speed_normalized = (static_cast<int>(car.speed) * 100) / static_cast<int>(normalized.speed);
    }

    uint8_t health = static_cast<uint8_t>(health_normalized);
    uint8_t handling = static_cast<uint8_t>(handling_normalized);
    uint8_t acceleration = static_cast<uint8_t>(accelaration_normalized);
    uint8_t speed = static_cast<uint8_t>(speed_normalized);

    return car_dto(car.model, health, handling, speed, acceleration);
}

void SelectCars::onCarChosen(const car_dto& car){
    actual_car = car;
    ui->Carinfo->clearContents();

    QFont itemFont;
    itemFont.setPointSize(14);
    itemFont.setBold(true);


    QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(car.model));
    nameItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    nameItem->setFont(itemFont);
    nameItem->setForeground(Qt::darkBlue);
    nameItem->setBackground(QColor(240, 240, 255));
    ui->Carinfo->setItem(0, 0, nameItem);

    car_dto normalized = inspect_catalogue();
    car_dto car_normalized = normalize_value_car(car, normalized);


    ui->Carinfo->setCellWidget(1, 0, createProgressBar(car_normalized.health));
    ui->Carinfo->setCellWidget(2, 0, createProgressBar(car_normalized.handling));
    ui->Carinfo->setCellWidget(3, 0, createProgressBar(car_normalized.acceleration));
    ui->Carinfo->setCellWidget(4, 0, createProgressBar(car_normalized.speed));
}

QWidget* SelectCars::createProgressBar(uint8_t value) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(20, 15, 20, 15);
    int value_bar = static_cast<int>(value);

    QProgressBar* progressBar = new QProgressBar();
    progressBar->setValue(value_bar);
    progressBar->setTextVisible(false);  // Ocultar el texto
    progressBar->setStyleSheet(getProgressBarStyle(value_bar));
    progressBar->setMinimumHeight(40);

    layout->addWidget(progressBar);

    return widget;
}

QString SelectCars::getProgressBarStyle(const int value) {
    QString color;

    if (value <= 45) {
        color = "#e74c3c";
    } else if (value <= 75) {
        color = "#f39c12";
    } else {
        color = "#2ecc71";
    }

    return QString(
                   "QProgressBar {"
                   "   border: 2px solid #7f8c8d;"
                   "   border-radius: 8px;"
                   "   background: #ecf0f1;"
                   "   height: 25px;"
                   "}"
                   "QProgressBar::chunk {"
                   "   background-color: %1;"
                   "   border-radius: 6px;"
                   "}"
                   ).arg(color);
}

void SelectCars::set_market(){
    ui->img_cars->setViewMode(QListWidget::IconMode);
    ui->img_cars->setFlow(QListWidget::LeftToRight);
    ui->img_cars->setWrapping(true);
    ui->img_cars->setResizeMode(QListWidget::Adjust);
    ui->img_cars->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    ui->img_cars->setSpacing(20);
    ui->img_cars->setIconSize(QSize(120, 120));
    ui->img_cars->clear();

    for (const car_dto& car : catalogue.cars) {
        QListWidgetItem* item = new QListWidgetItem();

        QString path = ":/cars/images/" + QString::fromStdString(car.model) + ".png";
        QPixmap pixmap(path);

        if (!pixmap.isNull()) {
            item->setIcon(QIcon(pixmap));
        }

        item->setText(QString::fromStdString(car.model));
        item->setTextAlignment(Qt::AlignCenter);
        item->setSizeHint(QSize(140, 140));

        ui->img_cars->addItem(item);
    }

    connect(ui->img_cars, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        int row = ui->img_cars->row(item);
        if (row >= 0 && row < catalogue.size) {
            this->onCarChosen(catalogue.cars[row]);
        }
    });

    ui->img_cars->setStyleSheet(
            "QListWidget {"
            "   background: transparent;"
            "   border: none;"
            "}"
            "QListWidget::item {"
            "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #34495e, stop:1 #2c3e50);"
            "   color: white;"
            "   font-weight: bold;"
            "   border-radius: 10px;"
            "   border: 2px solid #7f8c8d;"
            "   padding: 5px;"
            "}"
            "QListWidget::item:hover {"
            "   border: 2px solid #e74c3c;"
            "   background: #2c3e50;"
            "}"
            "QListWidget::item:selected {"
            "   border: 3px solid #e74c3c;"
            "   background: #2c3e50;"
            "}"
            );
}

void SelectCars::on_Select_clicked()
{
    //emit car selection
    emit confirm_car(actual_car);
}

