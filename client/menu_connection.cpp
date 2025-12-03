#include "menu_connection.h"

#include <QApplication>
#include <QString>
#include <QStringList>
#include <iostream>
#include <string>
#include <utility>

#include "Lobby/connectioninfo.h"

MenuConnection::MenuConnection() {}

std::pair<std::string, std::string> MenuConnection::run() {

    int fake_argc = 0;
    char* fake_argv[] = {nullptr};
    QApplication app(fake_argc, fake_argv);

    ConnectionInfo c;
    c.showMaximized();


    std::pair<std::string, std::string> result = std::pair<std::string, std::string>("", "");

    // connect confirm_connection signal
    QObject::connect(&c, &ConnectionInfo::emit_confirm_connection,
                     [&](const QString& ip, const QString& port) {
                         // check that the fields are not empty or with the correct format of ip or
                         // port
                         if (ip.isEmpty() || port.isEmpty()) {
                             // return empty strings to indicate failure
                             app.quit();
                             return result;
                         }

                         // check that port is a number
                         bool ok;
                         int port_num = port.toInt(&ok);
                         if (!ok || port_num < 1 || port_num > 65535) {
                             std::cout << "Invalid port number" << std::endl;
                             app.quit();
                             return result;
                         }

                         // check ip format (basic check)
                         QStringList ip_parts = ip.split(".");
                         if (ip_parts.size() != 4) {
                             std::cout << "Invalid IP address" << std::endl;
                             app.quit();
                             return result;
                         }
                         for (const QString& part: ip_parts) {
                             bool part_ok;
                             int part_num = part.toInt(&part_ok);
                             if (!part_ok || part_num < 0 || part_num > 255) {
                                 std::cout << "Invalid IP address" << std::endl;
                                 app.quit();
                                 return result;
                             }
                         }

                         // If we reach this point, the IP and port are valid
                         std::cout << "Connecting to IP: " << ip.toStdString()
                                   << " Port: " << port.toStdString() << std::endl;
                         result = std::pair<std::string, std::string>(ip.toStdString(),
                                                                      port.toStdString());
                         c.close();
                         app.quit();
                         return result;
                     });

    // connect close signal
    QObject::connect(&c, &ConnectionInfo::emit_close, [&]() { app.quit(); });

    app.exec();

    return result;
}
