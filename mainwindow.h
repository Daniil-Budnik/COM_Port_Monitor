#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QCustomPlot/qcustomplot.h>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QStatusBar>
#include <QComboBox>
#include <QString>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QtDebug>
#include <QTimer>
#include <QDateTime>
#include <QTime>
#include <QSpacerItem>

#include "Setting/settingwindow.h"
#include "Support/supportwindow.h"
#include "About/aboutwindow.h"
#include "News/newswindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    /* --- Кнопки меню программы --- */
    void on_UPDATE_clicked();           // Обновить список COM порт устройств
    void on_CONNECT_clicked();          // Подключить / Отключить соединение с COM порт устройством
    void on_START_clicked();            // Запустить  / Отключить чтение данных для отрисовки
    void on_STOP_clicked();             // Пауза / Продолжить
    void on_BYTE_clicked();             // Пропуск бита (для отладки или коллибровки в случае явной ошибки)

    /* --- Кнопки настроек вирутального COM порта  --- */
    void on_OK_BAUD_clicked();                      // Применить изминение физ. скорости в бодах
    void on_CH1_currentIndexChanged(int index);     // Кол-во байт для чтения в 1 канал
    void on_CH2_currentIndexChanged(int index);     // Кол-во байт для чтения в 2 канал
    void on_MODE_currentIndexChanged(int index);    // Режим чтения
    void on_FIRST_currentIndexChanged(int index);   // Чтение с старшего / младшего байта

    /* --- Кнопки на графике --- */
    void on_ZOOM_clicked();             // Переключатель: Авто масштабирование / Ручной режим
    void on_CLEAR_clicked();            // Очитка графика и данных
    void on_SCALE_clicked();            // Применить изминения параметром масштабирования

    /* --- Обработчики событий ---*/
    void ComEnable();                   // Проверка состояния COM порта
    void FrameUpDate();                 // Обновление графика

    /* --- Меню - File ---*/
    void on_B_SAVE_triggered();         // Сохранить базу данных графика
    void on_B_LOAD_triggered();         // Загрузить базу данных графика
    void on_B_OPTIONS_triggered();      // Окно настроек
    void on_B_EXIT_triggered();         // Выход

    /* --- Меню - Setting ---*/
    void on_B_TASK_triggered();         // Диспетчер задач
    void on_B_DEVICE_triggered();       // Диспетчер устройств
    void on_B_SYS_triggered();          // Информация о системе
    void on_B_CONTROL_triggered();      // Панель управления
    void on_B_OPTIONS_2_triggered();    // Настройки

    /* --- Меню - Help ---*/
    void on_B_ABOUT_triggered();        // О приложении
    void on_B_SUPPORT_2_triggered();    // Помощь



    void on_GRAPH_SCROLL_valueChanged(int value);

private:

    /* --- Другие окна --- */
    AboutWindow *aboutWindow        = new AboutWindow();
    SettingWindow *settingWindow    = new SettingWindow();
    SupportWindow *supportWindow    = new SupportWindow();
    NewsWindow *newsWindow          = new NewsWindow();

    /* --- Работа с графиками --- */
    Ui::MainWindow *ui;                         // Укзатель на графическую оболочку
    QCustomPlot *Graph;                         // Укзатель на объект графика

    void GraphDesign();                         // Диазйнер графиков
    void GraphInteractive(bool X);              // Интерактив с графиками

    /* --- Переменные для работы с графиком --- */
    int NowTime         = 0;
    double ySizeMin     = -500;
    double ySizeMax     = 500;
    double dT           = 100;
    double xScale       = 2000;


    /* --- Работа с виртуалным COM портами --- */
    QSerialPort *M_Serial = nullptr;            // Указатель на COM порт
    QLabel *STATUS, *VER;                       // Вывод информации в статус бар

    void OpenSerialPort();                      // Соединение с COM портом
    void WriteData(const QByteArray &data);     // Запись данных в COM порт
    void ReadData();                            // Чтение данных из COM порта

    /* --- Параметры виртуального COM порта --- */
    int BufSize = 0;
    int Baud = 19200;
    int BufSize_1 = 1;
    int BufSize_2 = 0;
    int BufMode = 0;
    int BufFirst = 0;

    /* --- Таймеры для обработки событий --- */
    QTimer *GridTimer, *ComTimer;


    /* --- Массивы для хранения данных графиков --- */
    QVector<QVector<double>> DATA = *new QVector<QVector<double>>();

    /* ---  Переключатели --- */
    bool PW_Connect = false;
    bool PW_Start   = false;
    bool PW_Pause   = false;
    bool PW_Rec     = false;
    bool PW_Zoom    = false;
    bool PW_COM     = false;

    /* ---  Конвертор данных ---*/
    volatile union{
        unsigned    char BYTE[8];
        unsigned    char uC[4];
        signed      char sC[4];
        unsigned    short uS;
        signed      short sS;
        unsigned    int   uI;
        signed      int   sI;
        unsigned    long  uL;
        signed      long  sL;
        float f;
        double d;
    } CONVERTER_DATA;

};
#endif // MAINWINDOW_H
