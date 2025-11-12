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
        LobbyDTO currentLobby = lobbies.lobbies[i];
        connect(joinButton, &QPushButton::clicked, this, [this, currentLobby]() {
            this->on_join_lobby(currentLobby);
        });
        ui->lobbytable->setCellWidget(i, 1, joinButton);
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


