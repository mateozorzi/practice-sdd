#ifndef QT_TEST_UTILS_H
#define QT_TEST_UTILS_H

#include <QApplication>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QWidget>
#include <functional>
#include <memory>

#include <gtest/gtest.h>

// Base for the suites that create windows: the QApplication lives while the suite runs (the
// menus under test create their own QApplication, so it cannot live for the whole process).
// Windows are drawn offscreen.
class QtTest: public ::testing::Test {
    static QApplication* app;

public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

// Closes every modal dialog the code under test opens (message boxes, file dialogs) and keeps
// the text of the message boxes. Needs a QApplication.
class ModalCloser: public QObject {
    QTimer timer;

public:
    QStringList messages;

    ModalCloser();
};

// Runs `step` repeatedly from the event loop until it returns true. Used to click through
// windows that block on their own event loop. Needs a QApplication.
class UiRobot: public QObject {
    QTimer timer;
    std::function<bool()> step;

public:
    explicit UiRobot(std::function<bool()> step);
};

// Visible top level window of type T, or nullptr
template <typename T>
T* visible_window() {
    for (QWidget* widget: QApplication::topLevelWidgets()) {
        T* window = qobject_cast<T*>(widget);
        if (window && window->isVisible())
            return window;
    }
    return nullptr;
}

// Calls `callback` from the constructor of the next QApplication (the menus create their own)
void on_next_application(std::function<void()> callback);

// Makes Qt draw offscreen. Called before creating any QApplication.
void use_offscreen_platform();

#endif
