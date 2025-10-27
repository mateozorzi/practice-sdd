#ifndef CREADOR_H
#define CREADOR_H

#include <QMainWindow>
#include "coordinates.h"
#include "messages.h"
#include <QGraphicsScene>
#include "checkpoint.h"

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
    Creador(const QString& path, const QString& path_mask, QWidget *parent = nullptr);
    void set_background(const QString& path);
    void handle_checkpoints(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event) override;
    ~Creador();

private:
    Ui::Creador *ui;
    QGraphicsScene* scene;
    QString map;
    QString mask;
    Coordinates checkpoints;
    Messages m;
    QImage mask_circuit;

    void add_checkpoint(QMouseEvent *event);
    void delete_checkpoint(QMouseEvent *event);
    void update_counter();
    void update_map(const QPoint& pos, int mode);
    void crear_csv();
    bool is_street(const QPoint& pos);
    Checkpoint calculate_checkpoint(const QPoint& pos);

signals:
    void close_w();
private slots:
    void on_Exit_clicked();
    void on_Done_clicked();
};

#endif // CREADOR_H
