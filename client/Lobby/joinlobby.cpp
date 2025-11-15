#include "joinlobby.h"

#include "ui_joinlobby.h"
#include "../../common/lobbies_catalogue_dto.h"

#include <QMessageBox>

JoinLobby::JoinLobby(LobbyCatalogueDTO lobbies, QWidget* parent):
        QMainWindow(parent), ui(new Ui::JoinLobby) {
    ui->setupUi(this);

    fill_table(lobbies);
}

void JoinLobby::on_join_lobby(const LobbyDTO& lobby){
    emit join_lobby(lobby);
}

void JoinLobby::fill_table(const LobbyCatalogueDTO& lobbies){
    ui->lobbytable->clear();

    ui->lobbytable->setRowCount(lobbies.size);
    QStringList colLabels;
    colLabels <<"id"<<"name"<<"active players"<<"max players"<<"action";
    ui->lobbytable->setColumnCount(colLabels.size());
    ui->lobbytable->setHorizontalHeaderLabels(colLabels);
    ui->lobbytable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lobbytable->horizontalHeader()->setVisible(true);
    ui->lobbytable->verticalHeader()->setVisible(false);

    for (int i = 0; i < lobbies.size; ++i) {
        const auto& lobby = lobbies.lobbies[i];

        QTableWidgetItem* items[] = {
                new QTableWidgetItem(QString::number(lobby.lobby_id)),
                new QTableWidgetItem(QString::fromStdString(lobby.lobby_name)),
                new QTableWidgetItem(QString::number(lobby.active_players)),
                new QTableWidgetItem(QString::number(lobby.max_players))
        };

        for (auto* item : items) {
            item->setTextAlignment(Qt::AlignCenter);
        }

        for (int col = 0; col < 4; ++col) {
            ui->lobbytable->setItem(i, col, items[col]);
        }

        // Botón de Join
        QPushButton *joinButton = new QPushButton("Join");
        joinButton->setStyleSheet("QPushButton {"
                                  "    background-color: #28a745;"
                                  "    color: white;"
                                  "    border: none;"
                                  "    padding: 8px 16px;"
                                  "    border-radius: 4px;"
                                  "    font-weight: bold;"
                                  "}");

        connect(joinButton, &QPushButton::clicked, this, [this, lobby]() {
            this->on_join_lobby(lobby);
        });
        ui->lobbytable->setCellWidget(i, 4, joinButton);
    }
}

void JoinLobby::join_denied(){
    QMessageBox::warning(this, "Access Denied", "Denied join to lobby");
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


