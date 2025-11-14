#ifndef SELECTCIRCUITS_H
#define SELECTCIRCUITS_H

#include <QMainWindow>

namespace Ui {
class SelectCircuits;
}

class SelectCircuits: public QMainWindow {
    Q_OBJECT

public:
    explicit SelectCircuits(QWidget* parent = nullptr);
    ~SelectCircuits();

private:
    Ui::SelectCircuits* ui;
};

#endif  // SELECTCIRCUITS_H
