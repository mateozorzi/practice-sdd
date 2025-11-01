#include "editormenu.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EditorMenu w;
    w.show();
    return a.exec();
}
