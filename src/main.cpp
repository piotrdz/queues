#include "ui/main_window.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication* app = new QApplication(argc, argv);

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    int exitCode = 0;
    exitCode = app->exec();

    delete mainWindow;
    delete app;

    return exitCode;
}

