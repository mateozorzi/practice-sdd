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

    //SelectCars c;
    //c.showMaximized();

    //creategame g;
    //g.showMaximized();

    //WaitingRoom w(true);
    //w.show();



    return a.exec();
}
