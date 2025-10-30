#include <QApplication>

#include "lobby.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    Lobby w;
    w.showMaximized();
    return a.exec();
}
