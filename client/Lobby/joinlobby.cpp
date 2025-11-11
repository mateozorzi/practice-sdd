#include "joinlobby.h"

#include "ui_joinlobby.h"
#include "../../common/lobbies_catalogue_dto.h"

JoinLobby::JoinLobby(LobbyCatalogueDTO lobbies, QWidget* parent):
        QMainWindow(parent), ui(new Ui::JoinLobby) {
    ui->setupUi(this);

    fill_table(lobbies);
}

void JoinLobby::on_join_lobby(const uint32_t id){
    emit join_lobby(id);
}

void JoinLobby::fill_table(const LobbyCatalogueDTO& lobbies){
    ui->lobbytable->clear();

    ui->lobbytable->setRowCount(lobbies.size);
    ui->lobbytable->setColumnCount(2);
    QStringList colLabels;
    colLabels <<"id"<<"action";
    ui->lobbytable->setHorizontalHeaderLabels(colLabels);
    ui->lobbytable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lobbytable->horizontalHeader()->setVisible(true);
    ui->lobbytable->verticalHeader()->setVisible(false);

    for (int i = 0; i < lobbies.size; ++i){
        QTableWidgetItem *item = new QTableWidgetItem();

        item->setText(QString::number(lobbies.lobbies[i].lobby_id));
        item->setTextAlignment(Qt::AlignCenter);
        ui->lobbytable->setItem(i, 0, item);

        QPushButton *joinButton = new QPushButton("Join");
        joinButton->setStyleSheet("QPushButton {"
                                  "    background-color: #28a745;"
                                  "    color: white;"
                                  "    border: none;"
                                  "    padding: 8px 16px;"
                                  "    border-radius: 4px;"
                                  "    font-weight: bold;"
                                  "}");
        uint32_t currentLobbyId = lobbies.lobbies[i].lobby_id;
        connect(joinButton, &QPushButton::clicked, this, [this, currentLobbyId]() {
            this->on_join_lobby(currentLobbyId);
        });
        ui->lobbytable->setCellWidget(i, 1, joinButton);
    }

}

JoinLobby::~JoinLobby() { delete ui; }

void JoinLobby::on_exit_clicked()
{
    emit close_menu();
}


void JoinLobby::on_refresh_clicked()
{
    emit refresh();
}


void JoinLobby::on_pushButton_clicked()
{
}

