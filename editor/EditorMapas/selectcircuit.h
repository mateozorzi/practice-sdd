#ifndef SELECTCIRCUIT_H
#define SELECTCIRCUIT_H

#include <QMainWindow>
#include "race.h"

namespace Ui {
class SelectCircuit;
}

class SelectCircuit : public QMainWindow
{
    Q_OBJECT

public:
    SelectCircuit(const QString& path, QWidget *parent = nullptr);
    ~SelectCircuit();

signals:
    void close_select_race();

private slots:
    void on_pushButton_clicked();


private:
    Ui::SelectCircuit *ui;
    QVector<Race> races;
    QString path_races;

    void delete_race(const QString& race_name);
    void fill_table();
    void read_info_races(const QStringList& races);
    void fill_races_info();
    void select_race(const Race& race);
};

#endif // SELECTCIRCUIT_H
