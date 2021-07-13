#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QMainWindow>

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

private:
    Ui::SettingWindow *ui;
};

#endif // SETTINGWINDOW_H
