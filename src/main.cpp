#include "engine/event.hpp"

#include "ui/main_window.hpp"
#include "ui/statistics_window.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(resources);

    qRegisterMetaType<Event>("Event");

    QApplication* app = new QApplication(argc, argv);

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    StatisticsWindow* statisticsWindow = new StatisticsWindow();
    statisticsWindow->show();

    mainWindow->setStatisticsWindow(statisticsWindow);

    int exitCode = 0;
    exitCode = app->exec();

    delete mainWindow;
    delete statisticsWindow;
    delete app;

    return exitCode;
}

