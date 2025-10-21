#ifndef CREADOR_H
#define CREADOR_H

#include <QMainWindow>

namespace Ui {
class Creador;
}

class Creador : public QMainWindow
{
    Q_OBJECT

public:
    Creador(const QString& path, QWidget *parent = nullptr);
    void colocar_fondo(const QString& path);
    ~Creador();

private:
    Ui::Creador *ui;
    QString mapa;

signals:
    void cerrado();
private slots:
    void on_pushButton_clicked();
};

#endif // CREADOR_H
