#ifndef SUPPORTWINDOW_H
#define SUPPORTWINDOW_H

#include <QMainWindow>

namespace Ui {
class SupportWindow;
}

class SupportWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SupportWindow(QWidget *parent = nullptr);
    ~SupportWindow();

private:
    Ui::SupportWindow *ui;
};

#endif // SUPPORTWINDOW_H
