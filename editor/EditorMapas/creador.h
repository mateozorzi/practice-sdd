#ifndef CREADOR_H
#define CREADOR_H

#include <QMainWindow>
#include "coordinates.h"
#include "messages.h"
#include <QGraphicsScene>

#define NONE 0
#define ADD 1
#define REMOVE 2

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
    QGraphicsScene* scene;
    QString map;
    Coordinates checkpoints;
    int mode;
    Messages m;

    void add_checkpoint(QEvent *event);
    void delete_checkpoint(QEvent *event);
    void update_counter();
    void update_map(const QPoint& pos);

signals:
    void close_w();
private slots:
    void on_Exit_clicked();
    void on_addButton_toggled(bool checked);
    void on_removeButton_toggled(bool checked);
};

#endif // CREADOR_H
