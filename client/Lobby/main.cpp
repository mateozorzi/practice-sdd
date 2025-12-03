#include <QApplication>

#include "../../common/car_catalogue_dto.h"
#include "../../common/car_dto.h"

#include "connectioninfo.h"
#include "creategame.h"
#include "createlobby.h"
#include "lobby.h"
#include "selectcars.h"
#include "waitingroom.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    Lobby w;
    w.show();

    // creategame g;
    // g.showMaximized();

    // WaitingRoom w(true);
    // w.show();

    // CreateLobby c;
    // c.show();

    ConnectionInfo c;
    c.showMaximized();

    return a.exec();
}
