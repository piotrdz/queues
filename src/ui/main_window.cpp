#include "ui/main_window.hpp"


#include "ui_main_window.h"


MainWindow::MainWindow()
 : m_ui(new Ui::MainWindow())
{
    m_ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete m_ui;
    m_ui = nullptr;
}

