#include "supportwindow.h"
#include "ui_supportwindow.h"

SupportWindow::SupportWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SupportWindow)
{
    ui->setupUi(this);
}

SupportWindow::~SupportWindow()
{
    delete ui;
}
