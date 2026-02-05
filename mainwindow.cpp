#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lsqDialog(nullptr)
{
    ui->setupUi(this);
    
    // Create the LSQDialog
    lsqDialog = new LSQDialog(this);
    
    // Connect the New Project action
    connect(ui->actionNewProject, &QAction::triggered, this, &MainWindow::onNewProject);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewProject()
{
    if (lsqDialog) {
        lsqDialog->show();
    }
}
