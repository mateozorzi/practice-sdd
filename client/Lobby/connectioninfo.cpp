#include "connectioninfo.h"

#include "ui_connectioninfo.h"

ConnectionInfo::ConnectionInfo(QWidget* parent): QMainWindow(parent), ui(new Ui::ConnectionInfo) {
    ui->setupUi(this);
}

ConnectionInfo::~ConnectionInfo() { delete ui; }

void ConnectionInfo::on_Confirm_clicked()
{
    emit emit_confirm_connection(ui->input_ip->text(), ui->input_port->text());
}


void ConnectionInfo::on_Exit_clicked()
{
    emit emit_close();
    this->close();
}

