#include "creador.h"

#include <QBrush>
#include <QDir>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QList>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPen>
#include <QPoint>
#include <algorithm>
#include <cmath>

#include <stdio.h>

#include "Constants.h"
#include "checkpoint.h"
#include "coordinates.h"
#include "messages.h"
#include "ui_creador.h"

Creador::Creador(const QString& path, const QString& path_mask, QWidget* parent):
        QMainWindow(parent),
        ui(new Ui::Creador),
        map(path),
        mask(path_mask),
        checkpoints(Coordinates()),
        m(Messages()),
        race(Race()) {
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->labelmap->setScene(scene);

    set_background(map);

    mask_circuit.load(mask);

    ui->n_checkpoints->display(0);


    ui->labelmap->installEventFilter(this);
}

Creador::Creador(const Race& r, QWidget* parent):
        QMainWindow(parent),
        ui(new Ui::Creador),
        map(r.map),
        mask(r.mask),
        checkpoints(r.checkpoints),
        m(Messages()),
        race(r) {

    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->labelmap->setScene(scene);

    set_background(map);

    mask_circuit.load(mask);

    ui->n_checkpoints->display(r.n_checkpoints);
    ui->n_laps->setValue(r.n_laps);
    ui->name_circuit->setText(r.name);

    ui->labelmap->installEventFilter(this);

    import_checkpoints();
}

void Creador::import_checkpoints() {
    // draw checkpoints on the map
    QVector<Checkpoint> v_checkpoints = checkpoints.get_checkpoints();

    for (Checkpoint c: v_checkpoints) {
        QRect r = c.rect;
        int id = c.id;
        draw_checkpoint(r, id);
    }
}

void Creador::set_background(const QString& path) {
    QPixmap mapPixmap(path);
    if (!mapPixmap.isNull()) {
        scene->clear();
        ui->labelmap->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        scene->addPixmap(mapPixmap);
        scene->setSceneRect(mapPixmap.rect());
    }
}

void Creador::update_counter() {
    QList<QGraphicsItem*> items = scene->items();
    int count = std::count_if(items.begin(), items.end(), [](QGraphicsItem* item) {
        return item && item->type() == QGraphicsRectItem::Type;
    });

    ui->n_checkpoints->display(count);
}

Checkpoint Creador::calculate_checkpoint(const QPoint& pos) {
    int id = static_cast<int>(ui->n_checkpoints->value() + 1);
    int left = pos.x();   // cppcheck-suppress duplicateAssignExpression
    int right = pos.x();  // cppcheck-suppress duplicateAssignExpression
    while (left > 0 && is_street(QPoint(left - 1, pos.y()))) left--;
    while (right < mask_circuit.width() - 1 && is_street(QPoint(right + 1, pos.y()))) right++;
    int horizontalLength = right - left;


    int up = pos.y();    // cppcheck-suppress duplicateAssignExpression
    int down = pos.y();  // cppcheck-suppress duplicateAssignExpression
    while (up > 0 && is_street(QPoint(pos.x(), up - 1))) up--;
    while (down < mask_circuit.height() - 1 && is_street(QPoint(pos.x(), down + 1))) down++;
    int verticalLength = down - up;

    if (horizontalLength >= MAX_RECT && verticalLength >= MAX_RECT) {
        // intersection checkpoint
        QRect rect(pos.x() - INTERSECTION_SIZE / 2, pos.y() - INTERSECTION_SIZE / 2,
                   INTERSECTION_SIZE, INTERSECTION_SIZE);
        return Checkpoint(id, INTERSECTION, pos, rect);

    } else if (horizontalLength < verticalLength) {
        // horizontal checkpoint
        int height = 15;

        QRect rect = QRect(left, pos.y() - height / 2, horizontalLength, height);
        return Checkpoint(id, HORIZONTAL, pos, rect);
    } else {
        // vertical checkpoint
        int width = 15;

        QRect rect = QRect(pos.x() - width / 2, up, width, verticalLength);
        return Checkpoint(id, VERTICAL, pos, rect);
    }
}

void Creador::draw_id_checkpoint(QRect& fig, const int id) {
    QGraphicsTextItem* text = scene->addText(QString::number(id));

    QFont font("Arial", 12, QFont::Bold);
    text->setFont(font);

    text->setDefaultTextColor(Qt::black);
    text->setZValue(2);

    qreal text_x = fig.x() + fig.width() / 2 - text->boundingRect().width() / 2;
    qreal text_y = fig.y() + fig.height() / 2 - text->boundingRect().height() / 2;
    text->setPos(text_x, text_y);
}

void Creador::draw_checkpoint(QRect& fig, const int id) {
    scene->addRect(fig, QPen(Qt::red), QBrush(Qt::yellow));

    draw_id_checkpoint(fig, id);
}

void Creador::update_id(const int id) {
    // delete every id from nexts checkpoints
    checkpoints.update_id(id);
    for (QGraphicsItem* item: scene->items()) {
        if (dynamic_cast<QGraphicsRectItem*>(item) || dynamic_cast<QGraphicsTextItem*>(item)) {
            scene->removeItem(item);
            delete item;
        }
    }

    import_checkpoints();
}

void Creador::update_map_delete(const QPoint& pos, const int id) {
    QPointF clickPos = pos;

    for (QGraphicsItem* item: scene->items()) {
        if (auto rect = dynamic_cast<QGraphicsRectItem*>(item)) {
            if (rect->contains(clickPos)) {

                QList<QGraphicsItem*> nearbyItems = scene->items(rect->sceneBoundingRect());
                for (QGraphicsItem* nearby: nearbyItems) {
                    if (auto text = dynamic_cast<QGraphicsTextItem*>(nearby)) {
                        if (text->toPlainText() == QString::number(id)) {
                            scene->removeItem(text);
                            delete text;
                            break;
                        }
                    }
                }

                scene->removeItem(item);
                delete item;
                break;
            }
        }
    }

    update_counter();
    update_id(id);
}

void Creador::update_map_add(const QPoint& pos) {
    Checkpoint checkpoint = calculate_checkpoint(pos);
    if (checkpoints.add_checkpoint(checkpoint)) {
        draw_checkpoint(checkpoint.rect, checkpoint.id);

    } else {
        m.checkpoint_message(this, "Error", "Can´t add checkpoint in (%1, %2)", pos);
    }

    update_counter();
}

bool Creador::is_street(const QPoint& pos) {
    QColor streetColor(255, 36, 250);

    QColor pixelColor = mask_circuit.pixelColor(pos);

    int tolerance = 30;
    bool isStreet = std::abs(pixelColor.red() - streetColor.red()) < tolerance &&
                    std::abs(pixelColor.green() - streetColor.green()) < tolerance &&
                    std::abs(pixelColor.blue() - streetColor.blue()) < tolerance;

    if (isStreet) {
        // street
        return true;
    }
    return false;
}

void Creador::add_checkpoint(QMouseEvent* event) {

    QPointF scenePos = ui->labelmap->mapToScene(event->pos());
    QPoint pos = scenePos.toPoint();

    if (is_street(pos)) {
        update_map_add(pos);
    } else {
        m.checkpoint_message(this, "Error", "Can´t add checkpoint in (%1, %2)", pos);
    }
}

void Creador::delete_checkpoint(QMouseEvent* event) {

    QPointF scenePos = ui->labelmap->mapToScene(event->pos());
    QPoint pos = scenePos.toPoint();

    Checkpoint del = checkpoints.delete_checkpoint(pos);

    if (del.click.x() != -1) {
        update_map_delete(del.click, del.id);
    } else {
        m.checkpoint_message(this, "Error", "No checkpoint in (%1, %2)", pos);
    }
}

void Creador::handle_checkpoints(QMouseEvent* event) {
    // handle leftbutton or right button
    if (event->button() == Qt::LeftButton) {
        add_checkpoint(event);
    } else if (event->button() == Qt::RightButton) {
        delete_checkpoint(event);
    }
}

bool Creador::eventFilter(QObject* obj, QEvent* event) {
    if (obj == ui->labelmap && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        this->handle_checkpoints(mouseEvent);
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}


Creador::~Creador() {
    qDeleteAll(scene->items());
    delete scene;
    delete ui;
}


void Creador::on_Exit_clicked() {
    emit close_w();
    this->close();
}

void Creador::on_Done_clicked() {
    // create csv with the checkpoints of the race

    if (!checkpoints.enough_checkpoints()) {
        // no enough checkpoints added for a race
        m.message_no_args(this, "Error",
                          "Not enough checkpoints.\n A race must have at least 5 checkpoints.");
        return;
    }

    int n_laps = ui->n_laps->value();
    if (n_laps <= 0) {
        m.message_no_args(this, "Error", "Minimum number of laps is 1.");
        return;
    }

    QString name_circuit = ui->name_circuit->text();
    if (name_circuit.length() < MIN_LENGTH) {
        m.message_no_args(
                this, "Error",
                "Fill the name of the circuit.\n The name must have at least five letters");
        return;
    }


    if (checkpoints.export_checkpoints(name_circuit, map, n_laps, this->mask, this)) {
        // file create succesfully
        if (name_circuit != race.name) {
            QString old_path = QCoreApplication::applicationDirPath() + "/../server/races/" +
                               race.name + ".csv";
            QFile old_file(old_path);
            if (old_file.exists()) {
                old_file.remove();
            }
        }
        m.message_no_args(this, "Success", "Congratulations! Creation complete");
        emit close_w();
        this->close();
    } else {
        m.message_no_args(this, "Error", "An error has occured.\n The circuit has not been save.");
    }
}
