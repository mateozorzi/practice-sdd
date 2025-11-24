#include <QApplication>

#include "editormenu.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    EditorMenu w;
    w.showMaximized();
    return a.exec();
}
