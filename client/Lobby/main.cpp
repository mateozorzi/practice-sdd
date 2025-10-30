#include <QApplication>

#include "lobby.h"
#include "selectcars.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    //Lobby w;
    //w.showMaximized();
    SelectCars c;
    c.showMaximized();
    return a.exec();
}
