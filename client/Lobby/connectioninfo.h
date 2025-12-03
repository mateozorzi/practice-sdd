#ifndef CONNECTIONINFO_H
#define CONNECTIONINFO_H

#include <QMainWindow>

namespace Ui {
class ConnectionInfo;
}

class ConnectionInfo: public QMainWindow {
    Q_OBJECT

public:
    explicit ConnectionInfo(QWidget* parent = nullptr);
    ~ConnectionInfo();

signals:
    void emit_confirm_connection(const QString& ip, const QString& port);
    void emit_close();

private slots:
    void on_Confirm_clicked();

    void on_Exit_clicked();

private:
    Ui::ConnectionInfo* ui;
};

#endif  // CONNECTIONINFO_H
