#ifndef EDITORMENU_H
#define EDITORMENU_H

#include <QMainWindow>

namespace Ui {
class EditorMenu;
}

class EditorMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorMenu(QWidget *parent = nullptr);
    ~EditorMenu();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

private:
    Ui::EditorMenu *ui;

    QString find_races_path();
};

#endif // EDITORMENU_H
