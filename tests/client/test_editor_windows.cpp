#include <QApplication>
#include <QDir>
#include <QFile>
#include <QGraphicsView>
#include <QImage>
#include <QLCDNumber>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTextStream>
#include <cstdlib>
#include <vector>

#include <gtest/gtest.h>

#include "creador.h"
#include "editor.h"
#include "editormenu.h"
#include "messages.h"
#include "qt_test_utils.h"
#include "selectcircuit.h"

// Unit tests of the map editor windows. The windows get a temporary races folder, so saving a
// race never writes into the installation folder (/var/nfs/races).

namespace {

const QString LIBERTY_MAP =
        ":/circuitos/imagenes/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Liberty City.png";
const QString LIBERTY_MASK = ":/mask/imagenes/001 - B_STREETS - Liberty City.png";

void click(QWidget& window, const char* button) { window.findChild<QPushButton*>(button)->click(); }

// Lets the windows closed with WA_DeleteOnClose be deleted
void process_deletions() {
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QApplication::processEvents();
}

bool is_street(const QImage& mask, int x, int y) {
    QColor color = mask.pixelColor(x, y);
    return std::abs(color.red() - 255) < 30 && std::abs(color.green() - 36) < 30 &&
           std::abs(color.blue() - 250) < 30;
}

// One street point in each 300x300 cell of the mask, so they are far from each other
std::vector<QPoint> street_points(size_t amount) {
    QImage mask(LIBERTY_MASK);
    std::vector<QPoint> points;
    for (int cell_y = 0; cell_y + 300 <= mask.height(); cell_y += 300)
        for (int cell_x = 0; cell_x + 300 <= mask.width(); cell_x += 300)
            for (int y = cell_y + 100; y < cell_y + 200 && points.size() < amount; y += 4) {
                bool found = false;
                for (int x = cell_x + 100; x < cell_x + 200 && !found; x += 4)
                    if (is_street(mask, x, y)) {
                        points.emplace_back(x, y);
                        found = true;
                    }
                if (found)
                    break;
            }
    return points;
}

QStringList race_files(const QTemporaryDir& races_dir) {
    return QDir(races_dir.path()).entryList(QDir::Files);
}

QStringList read_lines(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QTextStream(&file).readAll().split('\n', Qt::SkipEmptyParts);
}

QPoint non_street_point() {
    QImage mask(LIBERTY_MASK);
    for (int y = 0; y < mask.height(); y += 7)
        for (int x = 0; x < mask.width(); x += 7)
            if (!is_street(mask, x, y))
                return QPoint(x, y);
    return QPoint(0, 0);
}

class EditorWindowsTest: public QtTest {
protected:
    ModalCloser modals;
};

class CreadorTest: public EditorWindowsTest {
protected:
    QTemporaryDir races_dir;
    Creador creador{LIBERTY_MAP, LIBERTY_MASK, races_dir.path()};

    void SetUp() override { creador.showMaximized(); }

    // Clicks the map in the given map (scene) position
    void click_map(const QPoint& map_position, Qt::MouseButton button = Qt::LeftButton) {
        auto* view = creador.findChild<QGraphicsView*>("labelmap");
        QPoint view_position = view->mapFromScene(map_position);
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(view_position),
                          QPointF(view->mapToGlobal(view_position)), button, button,
                          Qt::NoModifier);
        creador.eventFilter(view, &event);
    }

    int checkpoints_shown() { return creador.findChild<QLCDNumber*>("n_checkpoints")->intValue(); }

    // Fills a valid race: the given checkpoints, 2 laps and the name "my circuit"
    void fill_valid_race(const std::vector<QPoint>& points) {
        for (const QPoint& point: points) click_map(point);
        creador.findChild<QSpinBox*>("n_laps")->setValue(2);
        creador.findChild<QLineEdit*>("name_circuit")->setText("my circuit");
    }
};

}  // namespace


TEST_F(EditorWindowsTest, MessagesAreShownInDialogs) {
    QWidget parent;
    Messages messages;

    messages.checkpoint_message(&parent, "Error", "Bad point (%1, %2)", QPoint(3, 4));
    messages.message_no_args(&parent, "Info", "Done");

    EXPECT_EQ(modals.messages, (QStringList{"Bad point (3, 4)", "Done"}));
}

TEST_F(EditorWindowsTest, MenuOpensTheEditorAndComesBack) {
    EditorMenu menu;
    menu.show();

    click(menu, "pushButton_2");
    Editor* editor = visible_window<Editor>();
    ASSERT_NE(editor, nullptr);
    EXPECT_FALSE(menu.isVisible());

    click(*editor, "Exit");
    process_deletions();
    EXPECT_TRUE(menu.isVisible());

    click(menu, "pushButton_3");  // quit: there is no event loop running, nothing happens
}

TEST_F(EditorWindowsTest, MenuOpensTheRaceListAndComesBack) {
    EditorMenu menu;
    menu.show();

    click(menu, "pushButton");
    SelectCircuit* races = visible_window<SelectCircuit>();
    ASSERT_NE(races, nullptr);

    click(*races, "pushButton");
    process_deletions();
    EXPECT_TRUE(menu.isVisible());
}

TEST_F(EditorWindowsTest, InstalledEditorUsesTheServerRacesFolder) {
    Editor editor;
    editor.show();
    click(editor, "mapa1");
    Creador* creador = visible_window<Creador>();
    ASSERT_NE(creador, nullptr);
    EXPECT_EQ(creador->races_folder(), "/var/nfs/races");
    click(*creador, "Exit");
    process_deletions();

    EditorMenu menu;
    menu.show();
    click(menu, "pushButton");
    SelectCircuit* races = visible_window<SelectCircuit>();
    ASSERT_NE(races, nullptr);
    EXPECT_EQ(races->races_folder(), "/var/nfs/races");
    click(*races, "pushButton");
    process_deletions();
}

TEST_F(EditorWindowsTest, EditorOpensTheCreatorOfEachCity) {
    for (const char* button: {"mapa1", "mapa2", "mapa3"}) {
        Editor editor;
        editor.show();
        bool editor_closed = false;
        QObject::connect(&editor, &Editor::close_editor, [&]() { editor_closed = true; });

        click(editor, button);
        Creador* creador = visible_window<Creador>();
        ASSERT_NE(creador, nullptr) << button;
        EXPECT_FALSE(editor.isVisible());

        click(*creador, "Exit");
        process_deletions();
        EXPECT_TRUE(editor_closed) << button;
    }
}


TEST_F(CreadorTest, StartsWithoutCheckpoints) {
    EXPECT_EQ(checkpoints_shown(), 0);
    EXPECT_TRUE(creador.findChild<QLineEdit*>("name_circuit")->text().isEmpty());
}

TEST_F(CreadorTest, ClickingAStreetAddsACheckpoint) {
    std::vector<QPoint> points = street_points(2);
    ASSERT_EQ(points.size(), 2u);

    click_map(points[0]);
    click_map(points[1]);

    EXPECT_EQ(checkpoints_shown(), 2);
    EXPECT_TRUE(modals.messages.isEmpty());
}

TEST_F(CreadorTest, CheckpointsCannotBeRepeated) {
    QPoint point = street_points(1).at(0);

    click_map(point);
    click_map(point);

    EXPECT_EQ(checkpoints_shown(), 1);
    ASSERT_EQ(modals.messages.size(), 1);
    EXPECT_TRUE(modals.messages[0].startsWith("Can´t add checkpoint"));
}

TEST_F(CreadorTest, CheckpointsMustBeOnTheStreet) {
    click_map(non_street_point());

    EXPECT_EQ(checkpoints_shown(), 0);
    ASSERT_EQ(modals.messages.size(), 1);
    EXPECT_TRUE(modals.messages[0].startsWith("Can´t add checkpoint"));
}

TEST_F(CreadorTest, RightClickDeletesTheCheckpoint) {
    std::vector<QPoint> points = street_points(3);
    for (const QPoint& point: points) click_map(point);

    click_map(points[1], Qt::RightButton);

    EXPECT_EQ(checkpoints_shown(), 2);
    EXPECT_TRUE(modals.messages.isEmpty());
}

TEST_F(CreadorTest, RightClickWithoutCheckpointShowsAnError) {
    click_map(street_points(1).at(0), Qt::RightButton);

    ASSERT_EQ(modals.messages.size(), 1);
    EXPECT_TRUE(modals.messages[0].startsWith("No checkpoint in"));
}

TEST_F(CreadorTest, OtherButtonsAndEventsAreIgnored) {
    click_map(street_points(1).at(0), Qt::MiddleButton);
    QEvent other(QEvent::Enter);
    creador.eventFilter(creador.findChild<QGraphicsView*>("labelmap"), &other);

    EXPECT_EQ(checkpoints_shown(), 0);
}

TEST_F(CreadorTest, DoneValidatesTheRace) {
    std::vector<QPoint> points = street_points(5);
    ASSERT_EQ(points.size(), 5u);

    click(creador, "Done");  // no checkpoints
    for (const QPoint& point: points) click_map(point);
    creador.findChild<QSpinBox*>("n_laps")->setValue(0);
    click(creador, "Done");  // no laps
    creador.findChild<QSpinBox*>("n_laps")->setValue(2);
    creador.findChild<QLineEdit*>("name_circuit")->setText("abc");
    click(creador, "Done");  // short name

    ASSERT_EQ(modals.messages.size(), 3);
    EXPECT_TRUE(modals.messages[0].startsWith("Not enough checkpoints"));
    EXPECT_EQ(modals.messages[1], "Minimum number of laps is 1.");
    EXPECT_TRUE(modals.messages[2].startsWith("Fill the name of the circuit"));
    EXPECT_TRUE(creador.isVisible());
    EXPECT_TRUE(race_files(races_dir).isEmpty());
}

TEST_F(CreadorTest, DoneSavesTheRaceInTheRacesFolder) {
    std::vector<QPoint> points = street_points(5);
    ASSERT_EQ(points.size(), 5u);
    fill_valid_race(points);

    click(creador, "Done");

    // ModalCloser rejects any other dialog, so the file is only written if no dialog was shown
    QStringList lines = read_lines(races_dir.filePath("my circuit.csv"));
    ASSERT_EQ(lines.size(), 9);
    EXPECT_EQ(lines[0], LIBERTY_MAP + "," + LIBERTY_MASK);
    EXPECT_EQ(lines[1], "City,n_laps");
    EXPECT_EQ(lines[2], "Liberty City,2");
    EXPECT_EQ(lines[3], "id, orientation, xCenter, yCenter, width, height");
    for (int i = 0; i < 5; ++i) {
        QStringList row = lines[4 + i].split(',');
        ASSERT_EQ(row.size(), 6) << lines[4 + i].toStdString();
        EXPECT_EQ(row[0].toInt(), i + 1);
        EXPECT_TRUE(row[1] == "H" || row[1] == "V" || row[1] == "I") << row[1].toStdString();
        int width = row[4].toInt();
        int height = row[5].toInt();
        QRect checkpoint(row[2].toInt() - width / 2, row[3].toInt() - height / 2, width, height);
        EXPECT_TRUE(checkpoint.adjusted(-1, -1, 1, 1).contains(points[i]))
                << "checkpoint " << i + 1 << " is not where it was clicked";
    }
    EXPECT_EQ(modals.messages, QStringList{"Congratulations! Creation complete"});
    EXPECT_FALSE(creador.isVisible());
}

TEST_F(CreadorTest, ExistingRaceIsNotOverwritten) {
    {
        QFile existing(races_dir.filePath("my circuit.csv"));
        ASSERT_TRUE(existing.open(QIODevice::WriteOnly | QIODevice::Text));
        existing.write("existing race\n");
    }
    fill_valid_race(street_points(5));

    click(creador, "Done");  // ModalCloser answers "No" to overwriting

    ASSERT_FALSE(modals.messages.isEmpty());
    EXPECT_TRUE(modals.messages[0].startsWith("Already exists a race with this name"));
    EXPECT_EQ(read_lines(races_dir.filePath("my circuit.csv")), QStringList{"existing race"});
    QStringList new_race = read_lines(races_dir.filePath("my circuit_1.csv"));
    ASSERT_EQ(new_race.size(), 9);
    EXPECT_EQ(new_race[2], "Liberty City,2");
}

TEST_F(EditorWindowsTest, CreatorOpensAnExistingRace) {
    QVector<Checkpoint> checkpoints = {
            Checkpoint(1, 'H', QPoint(100, 100), QRect(90, 95, 20, 10)),
            Checkpoint(2, 'V', QPoint(300, 100), QRect(295, 90, 10, 20))};
    Race race(0, "park day", "Liberty City", LIBERTY_MAP, LIBERTY_MASK, 2, checkpoints, 3);

    Creador creador(race, QTemporaryDir().path());

    EXPECT_EQ(creador.findChild<QLCDNumber*>("n_checkpoints")->intValue(), 2);
    EXPECT_EQ(creador.findChild<QSpinBox*>("n_laps")->value(), 3);
    EXPECT_EQ(creador.findChild<QLineEdit*>("name_circuit")->text(), "park day");
}


class SelectCircuitTest: public EditorWindowsTest {
protected:
    QTemporaryDir races_dir;

    void write_race(const QString& name, const QStringList& checkpoint_rows) {
        QFile file(races_dir.filePath(name));
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        out << LIBERTY_MAP << "," << LIBERTY_MASK << "\n";
        out << "City,n_laps\n";
        out << "Liberty City,3\n";
        out << "id, orientation, xCenter, yCenter, width, height\n";
        for (const QString& row: checkpoint_rows) out << row << "\n";
    }
};

TEST_F(SelectCircuitTest, ListsTheRacesInTheFolder) {
    write_race("first.csv", {"1,H,100,100,66,15", "2,V,300,100,15,66", "", "bad,row"});
    write_race("second.csv", {"1,H,100,100,66,15"});
    QFile notes(races_dir.filePath("notes.txt"));
    ASSERT_TRUE(notes.open(QIODevice::WriteOnly));

    SelectCircuit window(races_dir.path());

    auto* table = window.findChild<QTableWidget*>("tableRaces");
    ASSERT_EQ(table->rowCount(), 2);
    EXPECT_EQ(table->item(0, 0)->text(), "first");
    EXPECT_EQ(table->item(0, 1)->text(), "Liberty City");
    EXPECT_EQ(table->item(0, 2)->text(), "2");
    EXPECT_EQ(table->item(0, 3)->text(), "3");
    EXPECT_EQ(table->item(1, 2)->text(), "1");
}

TEST_F(SelectCircuitTest, OpensARaceInTheCreatorAndComesBack) {
    write_race("first.csv", {"1,H,100,100,66,15", "2,V,300,100,15,66"});
    SelectCircuit window(races_dir.path());
    window.show();

    auto* table = window.findChild<QTableWidget*>("tableRaces");
    qobject_cast<QPushButton*>(table->cellWidget(0, 4))->click();
    Creador* creador = visible_window<Creador>();
    ASSERT_NE(creador, nullptr);
    EXPECT_FALSE(window.isVisible());
    EXPECT_EQ(creador->findChild<QLineEdit*>("name_circuit")->text(), "first");

    click(*creador, "Exit");
    process_deletions();
    EXPECT_TRUE(window.isVisible());
    EXPECT_EQ(table->rowCount(), 1);
}

TEST_F(SelectCircuitTest, ExitEmitsClose) {
    SelectCircuit window(races_dir.path());
    bool closed = false;
    QObject::connect(&window, &SelectCircuit::close_select_race, [&]() { closed = true; });

    click(window, "pushButton");

    EXPECT_TRUE(closed);
    EXPECT_EQ(window.findChild<QTableWidget*>("tableRaces")->rowCount(), 0);
}

TEST_F(SelectCircuitTest, RenamedRaceReplacesItsFile) {
    write_race("first.csv", {"1,H,100,100,66,15", "2,V,300,100,15,66", "3,H,500,100,66,15",
                             "4,V,700,100,15,66", "5,H,900,100,66,15"});
    SelectCircuit window(races_dir.path());
    window.show();
    auto* table = window.findChild<QTableWidget*>("tableRaces");
    qobject_cast<QPushButton*>(table->cellWidget(0, 4))->click();
    Creador* creador = visible_window<Creador>();
    ASSERT_NE(creador, nullptr);

    creador->findChild<QLineEdit*>("name_circuit")->setText("second race");
    click(*creador, "Done");
    process_deletions();

    EXPECT_EQ(QDir(races_dir.path()).entryList(QDir::Files), QStringList{"second race.csv"});
    EXPECT_TRUE(window.isVisible());
    ASSERT_EQ(table->rowCount(), 1);
    EXPECT_EQ(table->item(0, 0)->text(), "second race");
}

TEST_F(SelectCircuitTest, DeleteRemovesTheRaceFromItsFolder) {
    write_race("first.csv", {"1,H,100,100,66,15", "2,V,300,100,15,66"});
    SelectCircuit window(races_dir.path());
    auto* table = window.findChild<QTableWidget*>("tableRaces");

    qobject_cast<QPushButton*>(table->cellWidget(0, 5))->click();
    process_deletions();

    EXPECT_FALSE(QFile::exists(races_dir.filePath("first.csv")));
    EXPECT_EQ(table->rowCount(), 0);
}
