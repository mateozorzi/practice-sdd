#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Editor;
}
QT_END_NAMESPACE

class Editor : public QMainWindow
{
    Q_OBJECT

public:
    Editor(QWidget *parent = nullptr);
    ~Editor();

signals:
    void close_editor();

private slots:
    void on_mapa1_clicked();

    void on_mapa2_clicked();

    void on_mapa3_clicked();

    void on_Exit_clicked();
private:
    Ui::Editor *ui;
};


#endif // EDITOR_H
