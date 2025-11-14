#include <QApplication>

#include "lobby.h"
#include "selectcars.h"
#include "creategame.h"
#include "waitingroom.h"
#include "createlobby.h"
#include "../../common/car_catalogue_dto.h"
#include "../../common/car_dto.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    Lobby w;
    w.show();

    std::vector<car_dto> cars = {// Name, Health, Handling, MaxSpeed, Acceleration
                                 {"Compact", 100, 90, 100, 80},    {"Sport", 80, 95, 150, 100},
                                 {"Convertible", 85, 90, 140, 95}, {"Sedan", 90, 85, 120, 75},
                                 {"Jeep", 150, 70, 85, 60},        {"Rally", 120, 80, 110, 70},
                                 {"Truck", 200, 50, 80, 40}};

    CarCatalogueDTO catalogue(std::move(cars));

    SelectCars c(catalogue);
    c.showMaximized();

    //creategame g;
    //g.showMaximized();

    //WaitingRoom w(true);
    //w.show();

    //CreateLobby c;
    //c.show();

    return a.exec();
}
