#include "qt_test_utils.h"

#include <QAbstractButton>
#include <QDialog>
#include <QMessageBox>
#include <QtGlobal>
#include <utility>

namespace {

int app_argc = 1;
char app_name[] = "nfs_client_tests";
char* app_argv[] = {app_name, nullptr};

std::function<void()> pending_callback;

void run_pending_callback() {
    if (!pending_callback)
        return;
    // Run once the application is fully built and its event loop is running
    std::function<void()> callback = std::move(pending_callback);
    pending_callback = nullptr;
    QTimer::singleShot(0, callback);
}

}  // namespace


QApplication* QtTest::app = nullptr;

void QtTest::SetUpTestSuite() {
    use_offscreen_platform();
    app = new QApplication(app_argc, app_argv);
}

void QtTest::TearDownTestSuite() {
    delete app;
    app = nullptr;
}


ModalCloser::ModalCloser() {
    connect(&timer, &QTimer::timeout, this, [this]() {
        QWidget* modal = QApplication::activeModalWidget();
        if (!modal)
            return;
        if (auto* box = qobject_cast<QMessageBox*>(modal)) {
            messages << box->text();
            // Answer "No" when asked, the code under test must not overwrite anything
            if (QAbstractButton* no = box->button(QMessageBox::No))
                no->click();
            else
                box->accept();
        } else if (auto* dialog = qobject_cast<QDialog*>(modal)) {
            dialog->reject();
        } else {
            modal->close();
        }
    });
    timer.start(10);
}


UiRobot::UiRobot(std::function<bool()> step): step(std::move(step)) {
    connect(&timer, &QTimer::timeout, this, [this]() {
        if (this->step())
            timer.stop();
    });
    timer.start(10);
}


void on_next_application(std::function<void()> callback) {
    static bool registered = false;
    if (!registered) {
        qAddPreRoutine(run_pending_callback);
        registered = true;
    }
    pending_callback = std::move(callback);
}

void use_offscreen_platform() { qputenv("QT_QPA_PLATFORM", "offscreen"); }
