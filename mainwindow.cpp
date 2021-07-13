#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    M_Serial(new QSerialPort(this))
{
    ui->setupUi(this);

    // Новости
    newsWindow->show();

    // Статус бар
    STATUS  = new QLabel("\tWaiting for connection... \t\t\t");
    VER     = new QLabel("\t\t\t Ver: alpha 0.6.2 \t\t\t Author: D.Budnik\t\t\t");
    ui->statusbar->addWidget(STATUS,  Qt::AlignLeft);       // Сообщения о состоянии
    ui->statusbar->addWidget(VER,  Qt::AlignRight);         // Сообщение о версии и авторе

    // Резервируем массивы для хранения данных в них
    for(int ID = 0; ID < 4; ID++){ DATA.append(*new QVector<double>()); }

    // Граф
    Graph = ui->GRAPH;                                      // Получили указатель на график
    Graph->setOpenGl(true, 16);                             // Мультисемплинг
    GraphDesign();                                          // Дизайнер графиков
    on_SCALE_clicked();                                     // Производим первичное масштабирование
    on_CLEAR_clicked();                                     // Первичная очистка

    // COM Порт
    on_UPDATE_clicked();                                    // Сканируем COM порт устройства

    // Обработчик события отрисовки графика в реальном времени
    GridTimer = new QTimer();
    GridTimer->setInterval(20);
    connect(GridTimer,SIGNAL(timeout()),this,SLOT(FrameUpDate()));

    // Обработчик отключения соединения с COM порт устройством
    ComTimer = new QTimer();
    ComTimer->setInterval(100);
    connect(ComTimer,SIGNAL(timeout()),this,SLOT(ComEnable()));
}

/* Диструктор */
MainWindow::~MainWindow(){ delete ui; }


/* ------ ------ ------ ------------ ------ */
/* ------ ------ РАБОТА С ГРАФИКОЙ ------ ------ */
/* ------ ------ ------ ------------ ------ */

/* Дизайнер графиков */
void MainWindow::GraphDesign(){

    // Градиент фона графика
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));

    // Градиент фона рамки графика
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));

    // Установка градиентов
    Graph->setBackground(plotGradient);
    Graph->axisRect()->setBackground(axisRectGradient);
    Graph->rescaleAxes();

    // Цвет линий на рамке
    Graph->xAxis->setBasePen(QPen(Qt::white, 1));
    Graph->yAxis->setBasePen(QPen(Qt::white, 1));

    // Цвет делений
    Graph->xAxis->setTickPen(QPen(Qt::white, 1));
    Graph->yAxis->setTickPen(QPen(Qt::white, 1));

    // Цвет доп. делений
    Graph->xAxis->setSubTickPen(QPen(Qt::white, 1));
    Graph->yAxis->setSubTickPen(QPen(Qt::white, 1));

    // Цвет текста на рамке
    Graph->xAxis->setTickLabelColor(Qt::white);
    Graph->yAxis->setTickLabelColor(Qt::white);

    // Сетка
    Graph->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    Graph->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));

    // Стиль доп. делений
    Graph->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    Graph->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));

    // Включить отображение доп. делений
    Graph->xAxis->grid()->setSubGridVisible(true);
    Graph->yAxis->grid()->setSubGridVisible(true);

    // ХЗ
    Graph->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    Graph->yAxis->grid()->setZeroLinePen(Qt::NoPen);

    // ХЗ
    Graph->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    Graph->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    // Текст
    Graph->xAxis->setTickLabelFont(QFont(QFont().family(), 10));
    Graph->yAxis->setTickLabelFont(QFont(QFont().family(), 10));

    // Добавляем два пространства для построения графиков
    Graph->addGraph(); Graph->addGraph();

    // Установка цвета этих графиков
    Graph->graph(0)->setPen(QPen(Qt::yellow));
    Graph->graph(1)->setPen(QPen(Qt::green));
}

/* Интерактив графиков */
void MainWindow::GraphInteractive(bool X){
    if(X){ Graph->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom |
                   QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables); }
    else{ Graph->setInteractions(QCP::iSelectLegend); }
}

/* ------ ------ ------ ------------ ------ */




/* ------ ------ ------ ------------ ------ */
/* ------ ------ ОБРАБОТКА ТАЙМЕРОВ ------ ------ */
/* ------ ------ ------ ------------ ------ */

/* ------ ------ ------ ------------ ------ */

/* Поток обработки */
void MainWindow::ComEnable(){ if(!M_Serial->isOpen()){ if(PW_Connect){ on_CONNECT_clicked(); } } }

/* Обновление фрэйма */
void MainWindow::FrameUpDate(){

    // Смотрим, что в буффере
    BufSize = M_Serial->bytesAvailable();
    if(BufSize > BufSize_1 + BufSize_2){
        BufSize = BufSize / (BufSize_1 + BufSize_2);
        for(int BF = 0; BF < BufSize; BF++){ ReadData(); }
    }

    // Если буффер пополнился, производим обновление
    if(BufSize_1 > 0){ Graph->graph(0)->setData(DATA.at(0), DATA.at(1)); }
    if(BufSize_2 > 0){ Graph->graph(1)->setData(DATA.at(2), DATA.at(3)); }

    // Двигаем масштаб при необходимости
    if(NowTime > 0 and NowTime >= ui->GRAPH_SCROLL->value()+dT){
        on_GRAPH_SCROLL_valueChanged(NowTime+1-dT);
    }

    Graph->replot();
}

/* ------ ------ ------ ------------ ------ */




/* ------ ------ ------ ------------ ------ */
/* ------ ------ COM ПОРТ ------ ------ */
/* ------ ------ ------ ------------ ------ */

/* Подключиться к COM порт устройству */
void MainWindow::OpenSerialPort(){

    // Резервируем переменну для название COM порта
    QString NamePort = "";

    // Если COM Порт открыт, то закрыть
    if (M_Serial->isOpen()){ M_Serial->close(); }

    // Поиск по выбранному COM порт устройству
    for(int i = 0; i < ui->COM->currentText().length(); i++){
        if(ui->COM->currentText()[i] == ' '){ break; }
        else{NamePort+=ui->COM->currentText()[i];}
    }

    // Устанавливаем название и физ. скорость
    M_Serial->setPortName(NamePort);
    M_Serial->setBaudRate(ui->BAUD->text().toInt());

    // Устанавливаем размер байта
    if(ui->READ_BYTE->currentText() == "8 Bit"){ M_Serial->setDataBits(QSerialPort::Data8);}
    else if (ui->READ_BYTE->currentText() == "6 Bit"){M_Serial->setDataBits(QSerialPort::Data6);}
    else if (ui->READ_BYTE->currentText() == "7 Bit"){M_Serial->setDataBits(QSerialPort::Data7);}
    else if (ui->READ_BYTE->currentText() == "5 Bit"){M_Serial->setDataBits(QSerialPort::Data5);}

    // Устанавливаем стоп бит
    if(ui->STOP_BYTE->currentText() == "1 Bit"){M_Serial->setStopBits(QSerialPort::OneStop);}
    else if (ui->STOP_BYTE->currentText() == "1.5 Bit"){M_Serial->setStopBits(QSerialPort::OneAndHalfStop);}
    else if (ui->STOP_BYTE->currentText() == "2 Bit"){M_Serial->setStopBits(QSerialPort::TwoStop);}

    // Устанавливаем четность
    if(ui->PARITY->currentText() == "None"){ M_Serial->setParity(QSerialPort::NoParity);}
    else if (ui->PARITY->currentText() == "Odd"){M_Serial->setParity(QSerialPort::OddParity);}
    else if (ui->PARITY->currentText() == "Even"){M_Serial->setParity(QSerialPort::EvenParity);}
    else if (ui->PARITY->currentText() == "Mark"){M_Serial->setParity(QSerialPort::MarkParity);}
    else if (ui->PARITY->currentText() == "Space"){M_Serial->setParity(QSerialPort::SpaceParity);}

    // Подключаемся к COM порту
    if (M_Serial->open(QIODevice::ReadWrite)) {
        STATUS->setText("\tEstablished connection with device " + M_Serial->portName() +"\t");
        ComTimer->start();  PW_COM = true;
    }

    // Если соединиться не получилось
    else {
        PW_COM = false;
        STATUS->setText("\tConnection error...\t");
        if (M_Serial->isOpen()){ M_Serial->close(); }
    }
}

/* Кол-во байт в Sample на 1-ый канал */
void MainWindow::on_CH1_currentIndexChanged(int index) { BufSize_1 = index + 1; }

/* Кол-во байт в Sample на 2-ый канал */
void MainWindow::on_CH2_currentIndexChanged(int index) { BufSize_2 = index; }

/* Режим чтения данных */
void MainWindow::on_MODE_currentIndexChanged(int index) { BufMode = index; }

/* Старший / младший байт */
void MainWindow::on_FIRST_currentIndexChanged(int index) { BufFirst = index; }

/* Изминение бод (физ. скорости UART) */
void MainWindow::on_OK_BAUD_clicked() { Baud = ui->BAUD->text().toInt(); }

/* Отправить байты в COM порт */
void MainWindow::WriteData(const QByteArray &data){M_Serial->write(data);}

/* ------ ------ ------ ------------ ------ */



/* ------ ------ ------ ------------ ------ */
/* ------ ------ Меню - File ------ ------ */
/* ------ ------ ------ ------------ ------ */

/* Сохранить базу данных графика */
void MainWindow::on_B_SAVE_triggered(){ qDebug() << "Сохранён: " , QFileDialog::getSaveFileName(); }

/* Загрузить базу данных графика */
void MainWindow::on_B_LOAD_triggered(){ qDebug() << "Выбран: " , QFileDialog::getOpenFileName(); }

/* Окно настроек */
void MainWindow::on_B_OPTIONS_triggered(){ settingWindow->show(); }

/* Выход */
void MainWindow::on_B_EXIT_triggered(){ this->~MainWindow(); }

/* ------ ------ ------ ------------ ------ */




/* ------ ------ ------ ------------ ------ */
/* ------ ------ Меню - Setting ------ ------ */
/* ------ ------ ------ ------------ ------ */

/* Диспетчер задач */
void MainWindow::on_B_TASK_triggered(){ QDesktopServices::openUrl(QUrl("file:///C:/Windows/System32/Taskmgr.exe", QUrl::TolerantMode)); }

/* Диспетчер устройств */
void MainWindow::on_B_DEVICE_triggered(){ QDesktopServices::openUrl(QUrl("file:///C:/Windows/System32/devmgmt.msc", QUrl::TolerantMode)); }

/* Информация о системе */
void MainWindow::on_B_SYS_triggered(){ QDesktopServices::openUrl(QUrl("file:///C:/Windows/System32/msinfo32.exe", QUrl::TolerantMode)); }

/* Панель управления */
void MainWindow::on_B_CONTROL_triggered(){ QProcess::startDetached("\"C:\\Windows\\System32\\control.exe\""); }

/* Настройки */
void MainWindow::on_B_OPTIONS_2_triggered(){ settingWindow->show(); }

/* ------ ------ ------ ------------ ------ */




/* ------ ------ ------ ------------ ------ */
/* ------ ------ Меню - Help ------ ------ */
/* ------ ------ ------ ------------ ------ */

/* О приложении */
void MainWindow::on_B_ABOUT_triggered(){ aboutWindow->show(); }

/* Помощь */
void MainWindow::on_B_SUPPORT_2_triggered(){ supportWindow->show(); }

/* ------ ------ ------ ------------ ------ */




/* ------ ------ ------ ------------ ------ */
/* ------ КНОПКИ УПРАВЛЕНИЯ В МЕНЮ  ------ */
/* ------ ------ ------ ------------ ------ */

/* Скинарование всех подключенных COM порт устройств */
void MainWindow::on_UPDATE_clicked(){
    // Очищаем старые список COM портов
    ui->COM->clear();

    // Получаем все подключенные COM порт устройства
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->COM->addItem( info.portName() + " : " + info.description());
    }
}

/* Подключение / Отключеие */
void MainWindow::on_CONNECT_clicked()
{
    // Если подключение не установено
    if(!PW_Connect){

        // Подключаемся к выбранному COM порт устройству
        OpenSerialPort();

        // Если подключение происходит
        if(PW_COM){

            // Меняем иконку и ключи
            ui->CONNECT->setIcon(QIcon(QPixmap(":/ico/ico/discon.png")));
            PW_Connect = true;

            // Разрешаем чтение данных
            ui->START->setEnabled(true);

            // Блокируем настройки COM порта
            ui->COM->setEnabled(false);
            ui->UPDATE->setEnabled(false);
            ui->BAUD->setEnabled(false);
            ui->OK_BAUD->setEnabled(false);
            ui->READ_BYTE->setEnabled(false);
            ui->STOP_BYTE->setEnabled(false);
            ui->PARITY->setEnabled(false);
        }
    // Если подключение установено
    }else{

        // Меняем иконку и ключи
        ui->CONNECT->setIcon(QIcon(QPixmap(":/ico/ico/usb.png")));
        PW_Connect = false;
        PW_COM = false;

        // Если старт нажат, отжимаем
        if(PW_Start){ on_START_clicked(); }

        // Выключаем элементы работы с данными
        ui->START->setEnabled(false);
        ui->STOP->setEnabled(false);
        ui->BYTE->setEnabled(false);

        // Разрешаем настройки COM порта
        ui->COM->setEnabled(true);
        ui->UPDATE->setEnabled(true);
        ui->BAUD->setEnabled(true);
        ui->OK_BAUD->setEnabled(true);
        ui->READ_BYTE->setEnabled(true);
        ui->STOP_BYTE->setEnabled(true);
        ui->PARITY->setEnabled(true);

        // Выводим соответствующее сообщение в консоль, разрываем соединеие и таймер обработки
        STATUS->setText("\tBreak connection...\t");
        if(ComTimer->isActive()){ ComTimer->stop(); }
        if(M_Serial->isOpen()){ M_Serial->disconnect(); M_Serial->close(); }
    }
}

/* Старт / Стоп для чтения и отрисовки данных */
void MainWindow::on_START_clicked()
{
    // Обработка старта
    if(!PW_Start){

        // Меняем иконку
        ui->START->setIcon(QIcon(QPixmap(":/ico/ico/stop.png")));

        // Включаем возможность работы с паузой и пропуском бита
        PW_Start = true;
        ui->STOP->setEnabled(PW_Start);
        ui->BYTE->setEnabled(PW_Start);

        // Очистка буффера
        M_Serial->clear();

        // Включаем обработку данных
        GridTimer->start();
    }

    // Обработка стопа
    else{

        // Если пазуа, отключаем
        if(PW_Pause){ on_STOP_clicked(); }

        // Меняем иконку
        ui->START->setIcon(QIcon(QPixmap(":/ico/ico/start.png")));

        // Отключаем возможность работы с паузой и пропуском бита
        PW_Start = false;
        ui->STOP->setEnabled(PW_Start);
        ui->BYTE->setEnabled(PW_Start);

        // Останавливаем обновление графика
        GridTimer->stop();
    }
}

/* Пауза / Продолжить для чтения и отрисовки данных */
void MainWindow::on_STOP_clicked()
{
    // Обработка паузы
    if(!PW_Pause){

        // Временно останавливаем обработку для чтения и отрисовки данных
        PW_Pause = true;
        GridTimer->stop();

        // Меняем иконку
        ui->STOP->setIcon(QIcon(QPixmap(":/ico/ico/start.png")));
    }

    // Обработка для продолжения
    else{

        // Очистка буффера
        M_Serial->clear();

        // Продолжаем обработку для чтения и отрисовки данных
        PW_Pause = false;
        GridTimer->start();

        // Меняем иконку
        ui->STOP->setIcon(QIcon(QPixmap(":/ico/ico/pause.png")));
    }
}

/* Пропуск байта  */
void MainWindow::on_BYTE_clicked() { M_Serial->read(new char[1], 1); }

/* ------ ------ ------ ------------ ------ */




/* ------ ------ ------ ------------ ------ */
/* ------ КНОПКИ УПРАВЛЕНИЯ НА ГРАФИКЕ  ------ */
/* ------ ------ ------ ------------ ------ */

/* Автомасштаб или режим интерактива */
void MainWindow::on_ZOOM_clicked(){

    // Если включен режим интерактива
    if(PW_Zoom){
        // Отключаем интерактив
        PW_Zoom = false;
        GraphInteractive(PW_Zoom);

        // Меняем иконку
        ui->ZOOM->setIcon(QIcon(QPixmap(":/ico/ico/zoom.png")));


        // Считаем крайнию точку для автомасштаба по оX
        if(NowTime > dT){ on_GRAPH_SCROLL_valueChanged((NowTime-(dT*0.8))); }
        else{ on_GRAPH_SCROLL_valueChanged(0); }


        // Производим автомасштаб по oY
        Graph->yAxis->setRange(ySizeMin,ySizeMax);
        Graph->replot();
    }

    // Если выключен режим интерактива
    else{
        // Включаем интерактив
        PW_Zoom = true;
        GraphInteractive(PW_Zoom);

        // Меняем иконку
        ui->ZOOM->setIcon(QIcon(QPixmap(":/ico/ico/inter.png")));
    }
}

/* Очистка графика */
void MainWindow::on_CLEAR_clicked() {

    // Очистка массивов
    for(int ID = 0; ID < 4; ID++){ DATA[ID].clear(); }

    // Масштабируем
    NowTime = 0;
    NowTime = 0;
    on_GRAPH_SCROLL_valueChanged(0);
    Graph->yAxis->setRange(ySizeMin,ySizeMax);

    // Устанавливаем массивы данных и обновляем график
    Graph->graph(0)->setData(DATA.at(0), DATA.at(1));
    Graph->graph(1)->setData(DATA.at(2), DATA.at(3));
    Graph->replot();
}

/* Масштабирование */
void MainWindow::on_SCALE_clicked(){

    // Получаем данные о масштабировании
    ySizeMin    = ui->YMIN->text().toDouble();
    ySizeMax    = ui->YMAX->text().toDouble();
    dT          = ui->INTERVAL->text().toDouble();
    xScale      = ui->LENGHT->text().toDouble();

    ui->GRAPH_SCROLL->setMaximum(xScale);

    // Считаем крайнию точку для автомасштаба по оX
    if(NowTime > dT){ on_GRAPH_SCROLL_valueChanged((NowTime-(dT*0.8))); }
    else{ on_GRAPH_SCROLL_valueChanged(0); }


    // Присваиваем полученные данные и обновляем график
    Graph->yAxis->setRange(ySizeMin,ySizeMax);
    Graph->replot();
}

/* Смещение по оX при перемещении ползунка */
void MainWindow::on_GRAPH_SCROLL_valueChanged(int value){
    Graph->xAxis->setRange(value, value +  dT);
    Graph->replot();
}

/* ------ ------ ------ ------------ ------ */




/* ------ ------ ------ ------------ ------ */
/* ------ ОБРАБОТКА ЧТЕНИЯ ДАННЫХ  ------ */
/* ------ ------ ------ ------------ ------ */

/* Чтение данных */
void MainWindow::ReadData()
{

    // =================================================================================================
    // РЕЖИМ ЗНАКОВЫХ И БЕЗ ЗНАКОВЫХ ЦЕЛЫХ ЧИСЕЛ
    // =================================================================================================
    if(BufMode == 0 or BufMode == 1){

        // ---------------------------------------------------------
        if(BufSize_1 == 1){
            M_Serial->read((char*)CONVERTER_DATA.sC , 1);
            DATA[0].append(NowTime++);
            if(BufMode == 1){       DATA[1].append(CONVERTER_DATA.sC[0]); }
            else if(BufMode == 0){  DATA[1].append(CONVERTER_DATA.uC[0]);}
        }
        // ---------------------------------------------------------
        if(BufSize_2 == 1){
            M_Serial->read((char*)CONVERTER_DATA.sC , 1);
            DATA[2].append(NowTime++);
            if(BufMode == 1){       DATA[3].append(CONVERTER_DATA.sC[0]); }
            else if(BufMode == 0){  DATA[3].append(CONVERTER_DATA.uC[0]);}
        }
        // ---------------------------------------------------------

        // ---------------------------------------------------------
        if(BufSize_1 == 2){
            char MyByte[2];
            M_Serial->read(MyByte , 2);

            if(BufFirst == 0){
                CONVERTER_DATA.uC[1] = MyByte[0];
                CONVERTER_DATA.uC[0] = MyByte[1];
            }else{
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[0];
            }

            DATA[0].append(NowTime++);
            if(BufMode == 1){       DATA[1].append(CONVERTER_DATA.sS); }
            else if(BufMode == 0){  DATA[1].append(CONVERTER_DATA.uS);}
        }
        // ---------------------------------------------------------
        if(BufSize_2 == 2){
            char MyByte[2];
            M_Serial->read(MyByte , 2);

            if(BufFirst == 0){
                CONVERTER_DATA.uC[1] = MyByte[0];
                CONVERTER_DATA.uC[0] = MyByte[1];
            }else{
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[0];
            }

            DATA[2].append(NowTime++);
            if(BufMode == 1){       DATA[3].append(CONVERTER_DATA.sS); }
            else if(BufMode == 0){  DATA[3].append(CONVERTER_DATA.uS);}
        }
        // ---------------------------------------------------------

        // ---------------------------------------------------------
        if(BufSize_1 == 3){
            char MyByte[3];
            M_Serial->read(MyByte , 3);
            if(BufFirst == 0){
                CONVERTER_DATA.uC[3] = 0;
                CONVERTER_DATA.uC[2] = MyByte[0];
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[2];
            }else{
                CONVERTER_DATA.uC[3] = 0;
                CONVERTER_DATA.uC[2] = MyByte[2];
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[0];
            }

            DATA[0].append(NowTime++);
            if(BufMode == 1){       DATA[1].append(CONVERTER_DATA.sL); }
            else if(BufMode == 0){  DATA[1].append(CONVERTER_DATA.uL);}
        }
        // ---------------------------------------------------------
        if(BufSize_2 == 3){
            char MyByte[3];
            M_Serial->read(MyByte , 3);
            if(BufFirst == 0){
                CONVERTER_DATA.uC[3] = 0;
                CONVERTER_DATA.uC[2] = MyByte[0];
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[2];
            }else{
                CONVERTER_DATA.uC[3] = 0;
                CONVERTER_DATA.uC[2] = MyByte[2];
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[0];
            }

            DATA[2].append(NowTime++);
            if(BufMode == 1){       DATA[3].append(CONVERTER_DATA.sL); }
            else if(BufMode == 0){  DATA[3].append(CONVERTER_DATA.uL);}
        }
        // ---------------------------------------------------------

        // ---------------------------------------------------------
        if(BufSize_1 == 4){
            char MyByte[4];
            M_Serial->read(MyByte , 4);
            if(BufFirst == 0){
                CONVERTER_DATA.uC[3] = MyByte[0];
                CONVERTER_DATA.uC[2] = MyByte[1];
                CONVERTER_DATA.uC[1] = MyByte[2];
                CONVERTER_DATA.uC[0] = MyByte[3];
            }else{
                CONVERTER_DATA.uC[3] = MyByte[3];
                CONVERTER_DATA.uC[2] = MyByte[2];
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[0];
            }

            DATA[0].append(NowTime++);
            if(BufMode == 1){       DATA[1].append(CONVERTER_DATA.sL); }
            else if(BufMode == 0){  DATA[1].append(CONVERTER_DATA.uL);}
        }
        // ---------------------------------------------------------
        if(BufSize_2 == 4){
            char MyByte[4];
            M_Serial->read(MyByte , 4);
            if(BufFirst == 0){
                CONVERTER_DATA.uC[3] = MyByte[0];
                CONVERTER_DATA.uC[2] = MyByte[1];
                CONVERTER_DATA.uC[1] = MyByte[2];
                CONVERTER_DATA.uC[0] = MyByte[3];
            }else{
                CONVERTER_DATA.uC[3] = MyByte[3];
                CONVERTER_DATA.uC[2] = MyByte[2];
                CONVERTER_DATA.uC[1] = MyByte[1];
                CONVERTER_DATA.uC[0] = MyByte[0];
            }

            DATA[2].append(NowTime++);
            if(BufMode == 1){       DATA[3].append(CONVERTER_DATA.sL); }
            else if(BufMode == 0){  DATA[3].append(CONVERTER_DATA.uL);}
        }
        // ---------------------------------------------------------

    }

    // =================================================================================================
    // РЕЖИМ ВЕЩЕСТВЕННЫХ ЧИСЕЛ ЧИСЕЛ
    // =================================================================================================
    else{

        // ---------------------------------------------------------
        if(BufMode == 2){

            char MyByte[4];

            if(BufSize_1 == 5){
                M_Serial->read(MyByte , 4);
                if(BufFirst == 0){
                    CONVERTER_DATA.uC[3] = MyByte[0];
                    CONVERTER_DATA.uC[2] = MyByte[1];
                    CONVERTER_DATA.uC[1] = MyByte[2];
                    CONVERTER_DATA.uC[0] = MyByte[3];
                }else{
                    CONVERTER_DATA.uC[3] = MyByte[3];
                    CONVERTER_DATA.uC[2] = MyByte[2];
                    CONVERTER_DATA.uC[1] = MyByte[1];
                    CONVERTER_DATA.uC[0] = MyByte[0];
                }

                DATA[0].append(NowTime++);
                DATA[1].append(CONVERTER_DATA.f);
            }

            if(BufSize_2 == 5){
                M_Serial->read(MyByte , 4);
                if(BufFirst == 0){
                    CONVERTER_DATA.uC[3] = MyByte[0];
                    CONVERTER_DATA.uC[2] = MyByte[1];
                    CONVERTER_DATA.uC[1] = MyByte[2];
                    CONVERTER_DATA.uC[0] = MyByte[3];
                }else{
                    CONVERTER_DATA.uC[3] = MyByte[3];
                    CONVERTER_DATA.uC[2] = MyByte[2];
                    CONVERTER_DATA.uC[1] = MyByte[1];
                    CONVERTER_DATA.uC[0] = MyByte[0];
                }

                DATA[2].append(NowTime++);
                DATA[3].append(CONVERTER_DATA.f);
            }

        }
        // ---------------------------------------------------------
        if(BufMode == 3){
            char MyByte[8];

            if(BufSize_1 == 5){
                M_Serial->read(MyByte , 8);
                if(BufFirst == 0){
                    CONVERTER_DATA.BYTE[7] = MyByte[0];
                    CONVERTER_DATA.BYTE[6] = MyByte[1];
                    CONVERTER_DATA.BYTE[5] = MyByte[2];
                    CONVERTER_DATA.BYTE[4] = MyByte[3];
                    CONVERTER_DATA.BYTE[3] = MyByte[4];
                    CONVERTER_DATA.BYTE[2] = MyByte[5];
                    CONVERTER_DATA.BYTE[1] = MyByte[6];
                    CONVERTER_DATA.BYTE[0] = MyByte[7];
                }else{
                    CONVERTER_DATA.BYTE[7] = MyByte[7];
                    CONVERTER_DATA.BYTE[6] = MyByte[6];
                    CONVERTER_DATA.BYTE[5] = MyByte[5];
                    CONVERTER_DATA.BYTE[4] = MyByte[4];
                    CONVERTER_DATA.BYTE[3] = MyByte[3];
                    CONVERTER_DATA.BYTE[2] = MyByte[2];
                    CONVERTER_DATA.BYTE[1] = MyByte[1];
                    CONVERTER_DATA.BYTE[0] = MyByte[0];
                }

                DATA[0].append(NowTime++);
                DATA[1].append((double)CONVERTER_DATA.d);
            }

            if(BufSize_2 == 5){
                M_Serial->read(MyByte , 8);
                if(BufFirst == 0){
                    CONVERTER_DATA.BYTE[7] = MyByte[0];
                    CONVERTER_DATA.BYTE[6] = MyByte[1];
                    CONVERTER_DATA.BYTE[5] = MyByte[2];
                    CONVERTER_DATA.BYTE[4] = MyByte[3];
                    CONVERTER_DATA.BYTE[3] = MyByte[4];
                    CONVERTER_DATA.BYTE[2] = MyByte[5];
                    CONVERTER_DATA.BYTE[1] = MyByte[6];
                    CONVERTER_DATA.BYTE[0] = MyByte[7];
                }else{
                    CONVERTER_DATA.BYTE[7] = MyByte[7];
                    CONVERTER_DATA.BYTE[6] = MyByte[6];
                    CONVERTER_DATA.BYTE[5] = MyByte[5];
                    CONVERTER_DATA.BYTE[4] = MyByte[4];
                    CONVERTER_DATA.BYTE[3] = MyByte[3];
                    CONVERTER_DATA.BYTE[2] = MyByte[2];
                    CONVERTER_DATA.BYTE[1] = MyByte[1];
                    CONVERTER_DATA.BYTE[0] = MyByte[0];
                }

                DATA[2].append(NowTime++);
                DATA[3].append((double)CONVERTER_DATA.d);
            }

        }
        // ---------------------------------------------------------

    }

    NowTime++;

}








