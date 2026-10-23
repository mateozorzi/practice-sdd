#include <QPoint>
#include <QRect>
#include <QVector>

#include <gtest/gtest.h>

#include "Constants.h"
#include "checkpoint.h"
#include "coordinates.h"
#include "race.h"

// Unit tests of the map editor model. Coordinates::export_checkpoints is tested through the
// creator windows (test_editor_windows.cpp), which save into a temporary races folder.

namespace {

// Checkpoint clicked in (x, y) covering a 20x20 square around the click
Checkpoint checkpoint_at(int id, int x, int y, char orientation = 'H') {
    return Checkpoint(id, orientation, QPoint(x, y), QRect(x - 10, y - 10, 20, 20));
}

Coordinates coordinates_with(int amount) {
    Coordinates coordinates;
    for (int i = 1; i <= amount; ++i) coordinates.add_checkpoint(checkpoint_at(i, i * 100, 50));
    return coordinates;
}

}  // namespace


TEST(EditorCheckpointTest, DefaultCheckpointIsEmpty) {
    Checkpoint checkpoint;

    EXPECT_EQ(checkpoint.id, 0);
    EXPECT_EQ(checkpoint.orientation, '\0');
    EXPECT_EQ(checkpoint.click, QPoint(-1, -1));
    EXPECT_TRUE(checkpoint.rect.isNull());
}

TEST(EditorCheckpointTest, ContainsThePointsInsideItsRect) {
    Checkpoint checkpoint = checkpoint_at(1, 100, 100);

    EXPECT_TRUE(checkpoint.contains(QPoint(100, 100)));
    EXPECT_TRUE(checkpoint.contains(QPoint(91, 109)));
    EXPECT_FALSE(checkpoint.contains(QPoint(120, 100)));
}

TEST(EditorCheckpointTest, EqualityUsesOrientationAndClick) {
    Checkpoint checkpoint = checkpoint_at(1, 100, 100, 'H');

    EXPECT_TRUE(checkpoint == checkpoint_at(7, 100, 100, 'H'));
    EXPECT_FALSE(checkpoint == checkpoint_at(1, 100, 100, 'V'));
    EXPECT_FALSE(checkpoint == checkpoint_at(1, 101, 100, 'H'));
}

TEST(EditorCheckpointTest, UpdateIdDecrementsIt) {
    Checkpoint checkpoint = checkpoint_at(3, 0, 0);

    checkpoint.update_id();

    EXPECT_EQ(checkpoint.id, 2);
}


TEST(EditorCoordinatesTest, StartsEmpty) {
    Coordinates coordinates;

    EXPECT_TRUE(coordinates.is_empty());
    EXPECT_TRUE(coordinates.get_checkpoints().isEmpty());
    EXPECT_FALSE(coordinates.enough_checkpoints());
}

TEST(EditorCoordinatesTest, AddsCheckpointsInOrder) {
    Coordinates coordinates;

    EXPECT_TRUE(coordinates.add_checkpoint(checkpoint_at(1, 100, 100)));
    EXPECT_TRUE(coordinates.add_checkpoint(checkpoint_at(2, 300, 100)));

    EXPECT_FALSE(coordinates.is_empty());
    ASSERT_EQ(coordinates.get_checkpoints().size(), 2);
    EXPECT_EQ(coordinates.last_checkpoint().id, 2);
}

TEST(EditorCoordinatesTest, RejectsRepeatedCheckpoints) {
    Coordinates coordinates;
    coordinates.add_checkpoint(checkpoint_at(1, 100, 100));

    EXPECT_FALSE(coordinates.add_checkpoint(checkpoint_at(2, 100, 100)));
    EXPECT_EQ(coordinates.get_checkpoints().size(), 1);
}

TEST(EditorCoordinatesTest, RejectsCheckpointsInsideAnotherOne) {
    Coordinates coordinates;
    coordinates.add_checkpoint(checkpoint_at(1, 100, 100));

    EXPECT_FALSE(coordinates.add_checkpoint(checkpoint_at(2, 105, 105, 'V')));
    EXPECT_TRUE(coordinates.add_checkpoint(checkpoint_at(2, 150, 150, 'V')));
}

TEST(EditorCoordinatesTest, NeedsAMinimumOfCheckpoints) {
    EXPECT_FALSE(coordinates_with(MIN_CHECKPOINTS - 1).enough_checkpoints());
    EXPECT_TRUE(coordinates_with(MIN_CHECKPOINTS).enough_checkpoints());
}

TEST(EditorCoordinatesTest, DeletesTheCheckpointUnderThePoint) {
    Coordinates coordinates = coordinates_with(3);

    Checkpoint deleted = coordinates.delete_checkpoint(QPoint(205, 45));

    EXPECT_EQ(deleted.id, 2);
    ASSERT_EQ(coordinates.get_checkpoints().size(), 2);
    EXPECT_EQ(coordinates.get_checkpoints()[1].id, 3);
}

TEST(EditorCoordinatesTest, DeletingAnEmptySpotReturnsADefaultCheckpoint) {
    Coordinates coordinates = coordinates_with(2);

    Checkpoint deleted = coordinates.delete_checkpoint(QPoint(1000, 1000));

    EXPECT_EQ(deleted.id, 0);
    EXPECT_EQ(coordinates.get_checkpoints().size(), 2);
}

TEST(EditorCoordinatesTest, UpdateIdRenumbersTheFollowingCheckpoints) {
    Coordinates coordinates = coordinates_with(4);
    coordinates.delete_checkpoint(QPoint(200, 50));

    coordinates.update_id(2);

    QVector<Checkpoint> checkpoints = coordinates.get_checkpoints();
    ASSERT_EQ(checkpoints.size(), 3);
    EXPECT_EQ(checkpoints[0].id, 1);
    EXPECT_EQ(checkpoints[1].id, 2);
    EXPECT_EQ(checkpoints[2].id, 3);
}

TEST(EditorCoordinatesTest, CanBeBuiltFromExistingCheckpoints) {
    QVector<Checkpoint> checkpoints = {checkpoint_at(1, 0, 0), checkpoint_at(2, 100, 0)};

    Coordinates coordinates(checkpoints);

    EXPECT_EQ(coordinates.get_checkpoints().size(), 2);
    EXPECT_EQ(coordinates.last_checkpoint().id, 2);
}


TEST(EditorRaceTest, DefaultRaceIsEmpty) {
    Race race;

    EXPECT_EQ(race.id, 0);
    EXPECT_TRUE(race.name.isEmpty());
    EXPECT_EQ(race.n_laps, 0);
    EXPECT_TRUE(race.checkpoints.is_empty());
}

TEST(EditorRaceTest, KeepsItsDataAndCheckpoints) {
    QVector<Checkpoint> checkpoints = {checkpoint_at(1, 0, 0), checkpoint_at(2, 100, 0)};

    Race race(4, "park day", "Liberty City", "map.png", "mask.png", 2, checkpoints, 3);

    EXPECT_EQ(race.id, 4);
    EXPECT_EQ(race.name, "park day");
    EXPECT_EQ(race.city, "Liberty City");
    EXPECT_EQ(race.map, "map.png");
    EXPECT_EQ(race.mask, "mask.png");
    EXPECT_EQ(race.n_checkpoints, 2);
    EXPECT_EQ(race.n_laps, 3);
    EXPECT_EQ(race.checkpoints.get_checkpoints().size(), 2);
}
