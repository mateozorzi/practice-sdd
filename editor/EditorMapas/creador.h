#ifndef CREADOR_H
#define CREADOR_H

#include <QMainWindow>
#include "coordinates.h"

namespace Ui {
class Creador;
}

class Creador : public QMainWindow
{
    Q_OBJECT

public:
    Creador(const QString& path, QWidget *parent = nullptr);
    void set_background(const QString& path);
    void handle_checkpoints(QEvent *event);
    bool eventFilter(QObject *obj, QEvent *event) override;
    ~Creador();

private:
    Ui::Creador *ui;
    QString map;
    Coordinates checkpoints;

signals:
    void close_w();
private slots:
    void on_Exit_clicked();
};

#endif // CREADOR_H
