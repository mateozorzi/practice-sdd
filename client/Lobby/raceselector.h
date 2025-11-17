#ifndef RACESELECTOR_H
#define RACESELECTOR_H

#include <QMainWindow>
#include "../../common/race_catalogue_dto.h"
#include "../../common/race_dto.h"

namespace Ui {
class RaceSelector;
}

class RaceSelector: public QMainWindow {
    Q_OBJECT

public:
    explicit RaceSelector(RaceCatalogueDTO& races, QWidget* parent = nullptr);
    ~RaceSelector();

signals:
    void close_menu();
    void confirm_races(std::vector<RaceDTO> races);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::RaceSelector* ui;
    RaceCatalogueDTO& races;

    std::vector<RaceDTO> available_races;
    std::vector<RaceDTO> selected_races;

    void select_race(int race_index);
    void delete_race(int race_index);

    void fill_available_races();

    void config_select_races();
    void config_available_races();

    void refresh_available_table();
    void refresh_selected_table();
};

#endif  // RACESELECTOR_H
